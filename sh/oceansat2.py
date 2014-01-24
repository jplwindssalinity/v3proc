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
import getpass
import glob
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

    common_dirs = {'hdf': "/u/polecat-r0/fore/ISRO/L1B",
            'l1b': "./L1B/",
            'l2b': "./L2B/",
            'log': "./log/",
            'out': "./outdir/",
            'podaac': "./podaac/",
            'ncep': "/u/potr-r0/fore/ECMWF/nwp1/",
            'ecmwf': "/u/potr-r0/fore/ECMWF/nwp3/",
            'work': "/dev/shm/" + getpass.getuser() + "/"
            }

    common_files = {'times': "/u/polecat-r0/fore/ISRO/orb_ele/{0}_orbit_ele/rev_tstart_teqa_long.txt",
             'xfactor': "/u/potr-r0/bawillia/OSCAT/potr-r1_data/tables/Walkout_adjust_T28Nov2012.dat",
             'cfg': {'template': "./cfg/ISRO-12.5km-slice-comp-rainnn.rdf",
                     'gen': "ISRO.rdf"
                     },
             }


    def __init__(self, year, rev, revtag=None, keep_log=False, level=0):
        '''
        Create a REV object.

        Keyword arguments
        year -- the rev's year
        rev -- the rev number (NNNNN_NNNNN)
        keep_log -- after processing, don't delete the log file
        level -- clean level
        '''
        # Make sure these objects are strings
        if revtag is None:
            revtag = rev

        self.year = str(year)
        self.file_rev = str(rev)
        self.rev = str(revtag)

        self.keep_log = keep_log
        self.level = level

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

        self._dirs = {}
        self._files = {}

        self.build()

    def __del__(self):
        if self._handler is not None:
            self.teardown()

    def build(self):
        """Create rev-specific data structure information."""

        # Build object configuration variables (call after changing an attribute
        log.debug("Building")

        # Define working and output directories
        self._work = {'/': os.path.join(self.common_dirs['work'], self.year,
            self.rev)}
        self._podaac = os.path.join(self.common_dirs['podaac'], self.year,
                self.rev, '')
        self._out = os.path.join(self.common_dirs['out'], self.year, self.rev,
                '')

        # Build the target NetCDF output file name based on the L1B HDF file
        self._locate_l1b_hdf()

        # Overwrite self.file_rev w/ self.rev (for revs with wrong ID)
        fn = self._l1bhdf.split(os.path.sep)[-1]
        fn = fn[0:13] + self.rev + fn[24:]

        self._files['l1bhdf'] = fn
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

    def process(self):
        '''Run processing for a complete REV.'''

        log.info(80*'=')
        log.info("Processing begun for %s", self.rev)
        log.info(80*'=')

        # Download and extract existing L1B and L2B HDF files
        log.debug("Staging")
        self._advertise("Staging")
        makedirs(self._work['/'])
        shutil.copy2(self._l1bhdf, os.path.join(self._work['/'],
            self._files['l1bhdf']))

        # Generate the rev directory structure.
        log.debug("Generating")
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

        # Convert L1B HDF files to local-format L1B.
        log.debug("Converting L1B HDF")
        self._advertise("Converting L1B HDF")

        logSubprocess(self._work['/'], "l1b_isro_to_l1b_v1.3", "-c",
                self._files['cfg'], "-hhbias", self._hbias, "-vvbias",
                self._vbias, '-xf_table', self.common_files['xfactor'])

        # Convert L1B to L2A.
        log.debug("Converting L1B")
        self._advertise("Converting L1B")
        logSubprocess(self._work['/'], "l1b_to_l2a", self._files['cfg'])

        # Convert ISRO L2A composites.
        log.debug("Converting ISRO L2A Composites")
        self._advertise("Converting L2A Composites")
        logSubprocess(self._work['/'], "l2a_fix_ISRO_composites", "-c",
                self._files['cfg'], "-o", self._files['flagged'], "-kp")

        # Convert L2A to L2B.
        log.debug("Converting L2A")
        self._advertise("Converting L2A")
        shutil.move(self._work['flagged'], self._work['L2A_FILE'])
        logSubprocess(self._work['/'], "l2a_to_l2b", self._files['cfg'])

        # Convert L2B to NetCDF.
        log.debug("Converting L2B")
        self._advertise("Converting L2B")
        logSubprocess(self._work['/'], "os2_l2b_to_netcdf", "--l1bhdf",
                self._files['l1bhdf'], "--nc", self._files['nc'], "--l2b",
                self._files['L2B_FILE'], '--times', self._times, '--hhbias',
                self._hbias, "--vvbias", self._vbias, '--xfact',
                self.common_files['xfactor'], '--revtag', self.rev)

        # Apply rain bias correct and set up ancillary data descriptors.
        log.debug("Fixing NC")
        self._advertise("Fixing NC")
        callMatlab("os2_netcdf_fixup('", self._work['nc'], "')")

        # Finalize data creation for PO.DAAC.
        log.debug("Finalizing")
        self._advertise("Finalizing")
        # Gzip the NetCDF file
        logSubprocess(self._work['/'], "gzip", "--best", self._files['nc'])

        # Compute md5 sum
        with open(self._work['md5'], 'w') as fd:
            process = subprocess.call(['md5sum', self._files['gz']], stdout=fd,
                    cwd=self._work['/'])


        # Delete intermediate files.
        log.debug("Cleaning")
        self._advertise("Cleaning")

        safe = ['md5', 'gz', 'cfg']

        if (self.level >= 10):
            safe.append('L2B_FILE')

        if (self.level >= 1):
            safe.append('calctimes')

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

        # Move generated files into output tree.
        log.debug("Moving")
        self._advertise("Moving")
        shutil.rmtree(self._out, ignore_errors=True)
        shutil.copytree(self._work['/'], self._out)

        shutil.rmtree(self._podaac, ignore_errors=True)
        makedirs(self._podaac)

#        for f in ['gz', 'md5']:
#            src = os.path.join(self._out, self._files[f])
#            dst = os.path.join(self._podaac, self._files[f])
#            os.link(src, dst)

    def update_ct_bias(self):

        log.info(80*'=')
        log.info("Processing begun for %s", self.rev)
        log.info(80*'=')

        logSubprocess(self._out, "gunzip", self._files['gz']);

        callMatlab("update_ct_bias('", self._files['gz'], "', '",
                "/home/werne/quikscat/QScatSim/mat/rel_bias_130212.mat", "')")

        # Gzip the NetCDF file
        logSubprocess(self._out, "gzip", "--best", self._files['nc'])

        # Compute md5 sum
        with open(os.path.join(self._out, self._files['md5']), 'w') as fd:
            process = subprocess.call(['md5sum', self._files['gz']], stdout=fd,
                    cwd=self._out)

        shutil.rmtree(self._podaac, ignore_errors=True)
        makedirs(self._podaac)

        for f in ['gz', 'md5']:
            src = os.path.join(self._out, self._files[f])
            dst = os.path.join(self._podaac, self._files[f])
            os.link(src, dst)

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

        def _get_nudge_file(dt, path_id, prefix):
            dt = time.localtime(dt)

            nudge_filename = prefix + time.strftime("%Y%j%H", dt)
            return os.path.join(cls.common_dirs[path_id], nudge_filename)

        def _get_ncep_file(dt):
            '''Create the corresponding SNWP1 filename.'''
            return _get_nudge_file(dt, 'ncep', 'SNWP1');
        def _get_ecmwf_file(dt):
            '''Create the corresponding SNWP3 filename.'''
            return _get_nudge_file(dt, 'ecmwf', 'SNWP3');

        ecmwf_file = _get_ecmwf_file(dt)
        nudge_file = _get_ncep_file(dt)

        if not os.path.exists(nudge_file):
            dt -= 6*60*60
            nudge_file = _get_ncep_file(dt)
            if not os.path.exists(nudge_file):
                dt += 12*60*60
                nudge_file = _get_ncep_file(dt)
                if not os.path.exists(nudge_file):
                    # Use ECMWF
                    nudge_file = ecmwf_file

        return nudge_file


    def _locate_l1b_hdf(self):
        '''Find this rev's L1B HDF file .'''
        # Build a glob search pattern for the L1B HDF file
        hdfdir = os.path.join(self.common_dirs['hdf'], self.year)
        base = "S1L1B" + self.year + "???_" + self.file_rev + ".h5"
        pattern = os.path.join(hdfdir, base)

        # Look for it
        match = glob.glob(pattern)
        if match == []:
            # Oops, couldn't be found
            log.error("Match not found for %s (%s)", pattern, self.file_rev)
            raise exceptions.IOError("No file matching " + pattern)

        self._l1bhdf = match[0]


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
    if len(job) == 2:
        year,revid = job
        rev = OceanSAT2Rev(year=year, rev=revid, level=1, keep_log=True)
    else:
        year,revid,revtag = job
        rev = OceanSAT2Rev(year=year, rev=revid, revtag=revtag, level=1)

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

    handler = logging.FileHandler("/dev/shm/processing.log")
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
    rev.process()


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
            cmd = tuple(line.strip().split())
            commands.append(cmd)
        runserver(jobs=commands, addr=addr)

    else:
        runclient(addr=addr)

if __name__ == '__main__':
    main()
