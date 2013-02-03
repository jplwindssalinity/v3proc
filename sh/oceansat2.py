#!/usr/bin/env python2.7
######################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION:
#
# File Name:     oceansat2.py
# Creation Date: 17 Jan 2013
#
# Copyright 2009-2012, by the California Institute of Technology.
# ALL RIGHTS RESERVED.  United States Government Sponsorship
# acknowledged. Any commercial use must be negotiated with the Office
# of Technology Transfer at the California Institute of Technology.
#
# This software may be subject to U.S. export control laws and
# regulations.  By accepting this document, the user agrees to comply
# with all U.S. export laws and regulations.  User has the
# responsibility to obtain export licenses, or other export authority
# as may be required before exporting such information to foreign
# countries or providing access to foreign persons.
######################################################################

'''
Libraries and functions to process Oceansat-II Scatterometer wind data.
'''

import argparse
import exceptions
import fcntl
import glob
import inspect
import logging
import math
import Queue
import os
import re
import select
import shutil
import socket
import subprocess
import sys
import threading
import time

from multiprocessing.managers import SyncManager

log = logging.getLogger(__name__)

sys_log = logging.getLogger(__name__ + '.sys')

# Default parameters
_MATLAB_DIRS = ["/u/potr-r0/fore/ISRO/mat",
        "/u/potr-r0/werne/quikscat/QScatSim/mat"]

# We use explicitly log entry/exit procedures instead of using a decorator so
# that ipython's introspection will give the right function signature

def logEntry():
    '''Add a function entrance to the logger.'''
    name = inspect.stack()[1][3]
    log.debug("Entering " + name + "()")


def logExit():
    '''Add a function exit to the logger.'''
    name = inspect.stack()[1][3]
    log.debug("Exiting  " + name + "()")

def callMatlab(*commands):
    '''Execute a sequence of Matlab commands.'''

    # Add the paths
    matstr = ''.join(["addpath('" + d + "');" for d in _MATLAB_DIRS])

    # Add in the commands
    matstr += ''.join(commands)

    # Call Matlab
    process = subprocess.Popen(['matlab-7.11', '-nodisplay'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT)

    log.info("%s", process.communicate(matstr)[0])

    if (process.returncode != 0):
        raise exceptions.Exception((process.returncode, "Matlab failed."))

def makedirs(dir):
    '''Create a directory tree, ignoring exceptions.'''
    try:
        os.makedirs(dir)
    except exceptions.OSError:
        sys.exc_clear()


def logSubprocess(dir, *args):
    '''Execute a subprocess and capture stdout/stderr in the system log.'''
    log.debug("Executing %s in %s", ' '.join(args), dir)
    process = subprocess.Popen(args, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, cwd=dir)

    # Set the process's stdout to be nonblocking
    fd = process.stdout.fileno()
    fcntl.fcntl(process.stdout, fcntl.F_SETFL, fcntl.fcntl(process.stdout,
        fcntl.F_GETFL) | os.O_NONBLOCK)

    # As long as the process is running, grab and log its output
    def logStdout():
        rdy, _, _ = select.select([fd], [], [], 0.1)
        if rdy == [fd]:
            log.info(process.stdout.read().strip())

    while process.poll() is None:
        logStdout()
    logStdout()

    if (process.returncode != 0):
        raise exceptions.Exception((process.returncode, args[0] + " failed."))


class OceanSAT2Rev(object):
    '''An object representing a single REV for Oceansat-2 Scatterometer.'''

    # The function call sequence in a standard processing run
    processing_sequence = ['stage', 'generate', 'l1bhdf_to_l1b', 'l1b_to_l2a',
            'fix_isro_composites', 'l2a_to_l2b', 'l2b_to_netcdf',
            'netcdf_fixup', 'podaac', 'clean', 'moveToOutput']

    common_dirs = {'hdf': "/u/polecat-r0/fore/ISRO/L1B",
            'l1b': "./L1B/",
            'l2b': "./L2B/",
            'log': "./log/",
            'out': "./outdir/",
            'podaac': "./podaac/",
            'ecmwf': "/u/potr-r0/fore/ECMWF/nwp1/",
            'work': "/dev/shm/"
            }

    common_files = {'times': "/u/polecat-r0/fore/ISRO/orb_ele/{0}_orbit_ele/rev_tstart_teqa_long.txt",
             'xfactor': "/u/potr-r0/bawillia/OSCAT/potr-r1_data/tables/Walkout_adjust_T28Nov2012.dat",
             'cfg': {'template': "./cfg/ISRO-12.5km-slice-comp-rainnn.rdf",
                     'gen': "ISRO.rdf"
                     },
             }


    def __init__(self, year, rev, keep_log=False):
        '''
        Create a REV object.

        Keyword arguments
        year -- the rev's year
        rev -- the rev number (NNNNN_NNNNN)
        keep_log -- after processing, don't delete the log file
        '''
        # Make sure these objects are strings
        self.year = str(year)
        self.rev = str(rev)

        self.keep_log = keep_log

        # Create the file logger
        logdir = os.path.join(self.common_dirs['log'], self.year, self.rev)
        makedirs(logdir)
        logfile = "{0}({1})-{2}".format(socket.gethostname(), str(os.getpid()),
                time.strftime('%Y%jT%H%M%S'))
        self._logfile = os.path.join(logdir, logfile)

        self._handler = logging.FileHandler(self._logfile)
        self._handler.setFormatter(logging.Formatter(
            fmt="%(levelname)s %(asctime)s.%(msecs)03d %(message)s",
            datefmt="%Y%jT%H:%M:%S"))
        log.addHandler(self._handler)
        if log.getEffectiveLevel() > logging.INFO:
            # Set minimum logging level
            log.setLevel(logging.INFO)

        logEntry()

        self._dirs = {}
        self._files = {}

        self.build()

        logExit()

    def __del__(self):
        if self._handler is not None:
            self.teardown()

    def build(self):
        '''Build object configuration variables (call after changing an
        attribute).'''
        logEntry()

        # Define working and output directories
        self._work = {'/': os.path.join(self.common_dirs['work'], self.year,
            self.rev)}
        self._podaac = os.path.join(self.common_dirs['podaac'], self.year,
                self.rev, '')
        self._out = os.path.join(self.common_dirs['out'], self.year, self.rev,
                '')

        # Build the target NetCDF output file name based on the L1B HDF file
        self._locate_l1b_hdf()
        self._files['l1bhdf'] = self._l1bhdf.split(os.path.sep)[-1]
        ncfile = self._files['l1bhdf']
        for old,new in [("h5$", "nc"), ("L1B", "l2b"), ("S1", "os2_")]:
            ncfile = re.sub(old, new, ncfile)

        # Define configuration files
        self._times = self.common_files['times'].format((self.year))
        self._files['calctimes'] = 'times.txt'
        self._files['cfg'] = self.common_files['cfg']['gen']
        self._files['flagged'] = "l2a_flagged.dat"
        self._files['gz'] = ncfile + ".gz"
        self._files['md5'] = ncfile + ".gz.md5"
        self._files['nc'] = ncfile

        # Extract file names from the template .rdf
        with open(self.common_files['cfg']['template'], 'r') as tmpl:
            for line in tmpl.readlines():
                p = [x.strip() for x in line.split('=')]
                if len(p) == 2:
                    k,v = p
                    if k in ['L1B_FILE', 'L2A_FILE', 'L2B_FILE', 'EPHEMERIS_FILE']:
                        self._files[k] = v

        # Cache working version of the file names
        for i,v in self._files.iteritems():
            self._work[i] = os.path.join(self._work['/'], v)

        # Bias corrections per S. Jaruwatanadilok 16 Jan 2013
        if self.rev <= "04796_04797":
            self._vbias, self._hbias = "-0.2023", "0.0235"
        else:
            self._vbias, self._hbias = "0.3036", "0.5296"

        logExit()

    def stage(self):
        '''Download and extract existing L1B and L2B HDF files.'''
        logEntry()
        self._advertise("Staging")
        makedirs(self._work['/'])
        shutil.copy2(self._l1bhdf, self._work['/'])
        logExit()

    def generate(self):
        '''Generate the rev directory structure.'''
        logEntry()
        self._advertise("Generating")
        # Extract orbit start and end time
        callMatlab("OS2L1B_Extract_Times('", self._work['l1bhdf'], "', '",
                self._work['calctimes'], "')")

        # Pull out important information from times file
        with open(self._work['calctimes'], 'r') as times:
            lines = times.readlines()
            sec_year = lines[0].strip()
            closest_nwp = lines[-1].strip()

        nudge_file = self._get_nudgefield(closest_nwp)

        # Create a "re.sub" filter to build this rev's .rdf
        cwd = os.getcwd() + '/'
        def xform(line):
            '''Insert configuration information into .rdf config file.'''
            line = re.sub("NUDGE_WINDFIELD_FILE *= DUMMY_FILENAME",
                    "NUDGE_WINDFIELD_FILE = " + nudge_file, line)
            line = re.sub("L1B_HDF_FILE *= DUMMY_FILENAME",
                    "L1B_HDF_FILE = " + self._work['l1bhdf'], line)
            line = re.sub("ATTEN_MAP_SEC_YEAR *= DUMMY",
                    "ATTEN_MAP_SEC_YEAR = " + sec_year, line)
            line = re.sub("\./", cwd, line)
            return line

        # Write out this rev's .rdf
        with open(self.common_files['cfg']['template'], 'r') as tmpl:
            with open(self._work['cfg'], 'w') as cfg:
                for line in tmpl.readlines():
                    cfg.write(xform(line))

        logExit()

    def l1bhdf_to_l1b(self):
        '''Convert L1B HDF files to local-format L1B.'''

        logEntry()
        self._advertise("Converting L1B HDF")

        logSubprocess(self._work['/'], "l1b_isro_to_l1b_v1.3", "-c",
                self._files['cfg'], "-hhbias", self._hbias, "-vvbias",
                self._vbias, '-xf_table', self.common_files['xfactor'])

        logExit()


    def l1b_to_l2a(self):
        '''Convert L1B to L2A.'''
        logEntry()
        self._advertise("Converting L1B")
        logSubprocess(self._work['/'], "l1b_to_l2a", self._files['cfg'])
        logExit()


    def fix_isro_composites(self):
        '''Convert ISRO L2A composites.'''
        logEntry()
        self._advertise("Converting L2A Composites")
        logSubprocess(self._work['/'], "l2a_fix_ISRO_composites", "-c",
                self._files['cfg'], "-o", self._files['flagged'], "-kp")

        logExit()

    def l2a_to_l2b(self):
        '''Convert L2A to L2B.'''
        logEntry()
        self._advertise("Converting L2A")
        shutil.move(self._work['flagged'], self._work['L2A_FILE'])
        logSubprocess(self._work['/'], "l2a_to_l2b", self._files['cfg'])

        logExit()

    def l2b_to_netcdf(self):
        '''Convert L2B to NetCDF.'''
        logEntry()
        self._advertise("Converting L2B")
        logSubprocess(self._work['/'], "os2_l2b_to_netcdf", "--l1bhdf",
                self._files['l1bhdf'], "--nc", self._files['nc'], "--l2b",
                self._files['L2B_FILE'], '--times', self._times, '--hhbias',
                self._hbias, "--vvbias", self._vbias, '--xfact',
                self.common_files['xfactor'])

        logExit()

    def netcdf_fixup(self):
        '''Apply rain bias correct and set up ancillary data descriptors.'''
        logEntry()
        self._advertise("Fixing NC")
        callMatlab("os2_netcdf_fixup('", self._work['nc'], "')")

        logExit()

    def podaac(self):
        '''Finalize data creation for PO.DAAC.'''
        logEntry()
        self._advertise("Finalizing")
        # Gzip the NetCDF file
        logSubprocess(self._work['/'], "gzip", "--best", self._files['nc'])

        # Compute md5 sum
        with open(self._work['md5'], 'w') as fd:
            process = subprocess.call(['md5sum', self._files['gz']], stdout=fd,
                    cwd=self._work['/'])

        logExit()

    def clean(self, level=0):
        '''Delete intermediate files.'''
        logEntry()
        self._advertise("Cleaning")

        safe = ['md5', 'gz', 'cfg']

        if (level >= 1):
            safe.append('L2B_FILE')

        for i,v in self._files.iteritems():
            if i not in safe:
                try:
                    os.remove(self._work[i])
                except exceptions.OSError as e:
                    sys.exc_clear()

        try:
            os.rmdir(self._work['/'])
        except exceptions.OSError as e:
            sys.exc_clear()

        logExit()

    def moveToOutput(self):
        '''Move generated files into output tree.'''
        logEntry()
        self._advertise("Moving")
        shutil.rmtree(self._out, ignore_errors=True)
        shutil.copytree(self._work['/'], self._out)

        shutil.rmtree(self._podaac, ignore_errors=True)
        makedirs(self._podaac)

        for f in ['gz', 'md5']:
            src = os.path.join(self._out, self._files[f])
            dst = os.path.join(self._podaac, self._files[f])
            os.link(src, dst)

        logExit()


    def process(self):
        '''Run processing for a complete REV.'''

        log.info(80*'=')
        log.info("Processing begun for %s", self.rev)
        log.info(80*'=')

        logEntry()
        self._advertise("Processing")

        [getattr(self, f)() for f in self.processing_sequence]

        logExit()

    def _advertise(self, string):
        '''Print a string with rev and time info.'''
        print ' '.join([string, self.rev, time.strftime("%Y%jT%H%M%S")])


    def teardown(self):
        log.removeHandler(self._handler)
        self._handler.close()
        self._handler = None
        shutil.rmtree(self._work['/'], ignore_errors=True)
        if not self.keep_log:
            os.remove(self._logfile)


    @classmethod
    def _get_nudgefield(cls, nudge):
        '''
        Return the filename for the closest nudge field file.

        Keyword arguments:
        nudge -- the desired YYYYDDDHH (w/ 06 | HH)
        '''
        log.debug(nudge)
        dt = time.strptime(nudge, "%Y%j%H")
        dt = time.mktime(dt)
        sign, delta = -1, 6

        def _get_ncep_file(dt):
            '''Create the corresponding SNWP1 filename.'''
            dt = time.localtime(dt)

            ncep_filename = "SNWP1" + time.strftime("%Y%j%H", dt)
            return os.path.join(cls.common_dirs['ecmwf'], ncep_filename)

        # 201122718
        while not os.path.exists(_get_ncep_file(dt)):
            # As long as the file doesn't exist, we need to update dt:
            # Example (w/ dt^ as the "target" time):
            # dt <- dt + -6  (in hours) (== dt^ -6)
            # dt <- dt + +12 (in hours) (== dt^ +6)
            # dt <- dt + -18 (in hours) (== dt^ -12)
            # dt <- dt + +24 (in hours) (== dt^ +12), ...
            dt += sign*delta*60*60
            sign *= -1
            delta += 6

        return _get_ncep_file(dt)


    def _locate_l1b_hdf(self):
        '''Find this rev's L1B HDF file .'''
        logEntry()
        # Build a glob search pattern for the L1B HDF file
        hdfdir = os.path.join(self.common_dirs['hdf'], self.year)
        base = "S1L1B" + self.year + "???_" + self.rev + ".h5"
        pattern = os.path.join(hdfdir, base)

        # Look for it
        match = glob.glob(pattern)
        if match == []:
            # Oops, couldn't be found
            log.error("Match not found for %s (%s)", pattern, self.rev)
            raise exceptions.IOError("No file matching " + pattern)

        self._l1bhdf = match[0]
        logExit()


def make_server(addr=('', 10000), authkey=''):
    class Server(SyncManager): pass

    j = Queue.Queue()
    l = threading.Lock()
    Server.register('jobs', callable=lambda: j)
    Server.register('log', callable=lambda: l)

    return Server(addr, authkey)

def log_call(log_call):
    def doit(text):
        log_call(text)
        map(lambda x: x.flush(), log_call.__self__.handlers)
    return doit

class dummyServer():
    def log(self):
        return threading.Lock()

def runjob(job, server=None):
    year,revid = job
    rev = OceanSAT2Rev(year=year, rev=revid)

    if server is None:
        server = dummyServer()

    try:
        print job
        rev.process()
        call_with_lock(log_call(sys_log.info), server.log(),
                ': '.join([revid, 'success']))
    except Exception as e:
        rev.keep_log = True
        call_with_lock(log_call(sys_log.error), server.log(),
                ': '.join([revid, str(e)]))
        # Deal with errno ENOSPC

    rev.teardown()


def runclient(addr=('', 10000)):
    setup_logs()

    server = make_server(addr=addr)
    server.connect()
    jobs_q = server.jobs()

    while not jobs_q.empty():
        job = jobs_q.get()
        runjob(job, server)
        jobs_q.task_done()


def runserver(jobs, addr=('', 10000)):

    server = make_server(addr=addr)

    server.start()
    jobs_q = server.jobs()

    for c in jobs:
        jobs_q.put(c)

    jobs_q.join()
    server.shutdown()


def setup_logs():
    class HostnameFilter(logging.Filter):
        hostname = socket.gethostname()
        def filter(self, record):
            record.hostname = self.hostname
            return True

    handler = logging.FileHandler("./processing.log")
    handler.setFormatter(logging.Formatter(
        fmt="%(levelname)s %(hostname)s(%(process)d) %(asctime)s.%(msecs)03d %(message)s",
        datefmt="%Y%jT%H:%M:%S"))
    sys_log.addFilter(HostnameFilter())
    sys_log.addHandler(handler)
    sys_log.setLevel(logging.INFO)


def call_with_lock(fcall, lock, text=''):
    print "Logging"
    lock.acquire()
    fcall(text)
    lock.release()


def test():
    logging.basicConfig(level=logging.DEBUG)
    rev = OceanSATRev(year=2010, rev='03297_03298')
    rev.all()


def main():
    parser = argparse.ArgumentParser(description='Process Oceansat-II Scatterometer Winds.')
    parser.add_argument('--file', default=None, type=file, help='config file')
    parser.add_argument('--server', help='server:port')
    args = parser.parse_args()

    host,port = args.server.split(':')
    addr = (host, int(port))
    print addr

    if args.file is not None:
        commands = []
        for line in args.file:
            x,y = line.strip().split()
            commands.append((x,y))
        runserver(jobs=commands, addr=addr)

    else:
        runclient(addr=addr)

if __name__ == '__main__':
    main()
