
#include <vector>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "L2BTBOnly.h"

L2BTBOnly::L2BTBOnly(const char* filename) : nati(0), ncti(0) {
    hid_t id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

    H5LTget_attribute_int(id, "/", "Number of Cross Track Bins", &ncti);
    H5LTget_attribute_int(id, "/", "Number of Along Track Bins", &nati);

    printf("%d %d\n", ncti, nati);

    _Allocate();
    H5LTread_dataset_float(id, "/anc_dir", &anc_dir[0]);
    H5LTread_dataset_float(id, "/anc_spd", &anc_spd[0]);
    H5LTread_dataset_float(id, "/anc_sss", &anc_sss[0]);
    H5LTread_dataset_float(id, "/anc_sst", &anc_sst[0]);
    H5LTread_dataset_float(id, "/anc_swh", &anc_swh[0]);
    H5LTread_dataset_float(id, "/azi_aft", &azi_aft[0]);
    H5LTread_dataset_float(id, "/azi_fore", &azi_fore[0]);
    H5LTread_dataset_float(id, "/inc_aft", &inc_aft[0]);
    H5LTread_dataset_float(id, "/inc_fore", &inc_fore[0]);
    H5LTread_dataset_float(id, "/lat", &lat[0]);
    H5LTread_dataset_float(id, "/lon", &lon[0]);
    H5LTread_dataset(id, "/n_h_aft", H5T_NATIVE_UCHAR, &n_h_aft[0]);
    H5LTread_dataset(id, "/n_h_fore", H5T_NATIVE_UCHAR, &n_h_fore[0]);
    H5LTread_dataset(id, "/n_v_aft", H5T_NATIVE_UCHAR, &n_v_aft[0]);
    H5LTread_dataset(id, "/n_v_fore", H5T_NATIVE_UCHAR, &n_v_fore[0]);
    H5LTread_dataset_float(id, "/nedt_h_aft", &nedt_h_aft[0]);
    H5LTread_dataset_float(id, "/nedt_h_fore", &nedt_h_fore[0]);
    H5LTread_dataset_float(id, "/nedt_v_aft", &nedt_v_aft[0]);
    H5LTread_dataset_float(id, "/nedt_v_fore", &nedt_v_fore[0]);
    H5LTread_dataset(id, "/quality_flag", H5T_NATIVE_USHORT, &quality_flag[0]);
    H5LTread_dataset_float(id, "/tb_h_aft", &tb_h_aft[0]);
    H5LTread_dataset_float(id, "/tb_h_fore", &tb_h_fore[0]);
    H5LTread_dataset_float(id, "/tb_v_aft", &tb_v_aft[0]);
    H5LTread_dataset_float(id, "/tb_v_fore", &tb_v_fore[0]);
    H5LTread_dataset_float(id, "/tb_v_bias_adj", &tb_v_bias_adj[0]);
    H5LTread_dataset_float(id, "/tb_h_bias_adj", &tb_h_bias_adj[0]);
    H5LTread_dataset_float(id, "/smap_spd", &smap_spd[0]);
    H5LTread_dataset_float(id, "/smap_sss", &smap_sss[0]);
    H5LTread_dataset_float(id, "/smap_spd_bias_adj", &smap_spd_bias_adj[0]);
    H5LTread_dataset_float(id, "/smap_sss_bias_adj", &smap_sss_bias_adj[0]);

    H5Fclose(id);
    return;
}

L2BTBOnly::~L2BTBOnly() {
    return;
}

int L2BTBOnly::_Allocate() {

    if(nati == 0 || ncti == 0)
        return 0;

    int l2b_size = nati * ncti;

    anc_dir.resize(l2b_size);
    anc_spd.resize(l2b_size);
    anc_sss.resize(l2b_size);
    anc_sst.resize(l2b_size);
    anc_swh.resize(l2b_size);
    azi_aft.resize(l2b_size);
    azi_fore.resize(l2b_size);
    inc_aft.resize(l2b_size);
    inc_fore.resize(l2b_size);
    lat.resize(l2b_size);
    lon.resize(l2b_size);
    n_h_aft.resize(l2b_size);
    n_h_fore.resize(l2b_size);
    n_v_aft.resize(l2b_size);
    n_v_fore.resize(l2b_size);
    nedt_h_aft.resize(l2b_size);
    nedt_h_fore.resize(l2b_size);
    nedt_v_aft.resize(l2b_size);
    nedt_v_fore.resize(l2b_size);
    quality_flag.resize(l2b_size);
    tb_h_aft.resize(l2b_size);
    tb_h_fore.resize(l2b_size);
    tb_v_aft.resize(l2b_size);
    tb_v_fore.resize(l2b_size);
    tb_v_bias_adj.resize(l2b_size);
    tb_h_bias_adj.resize(l2b_size);
    smap_spd.resize(l2b_size);
    smap_sss.resize(l2b_size);
    smap_spd_bias_adj.resize(l2b_size);
    smap_sss_bias_adj.resize(l2b_size);

    return(1);
}
