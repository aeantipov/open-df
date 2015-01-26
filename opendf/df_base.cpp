#include "df_base.hpp"
#include "diagrams.hpp"
#include "lattice_traits.hpp"

namespace open_df { 

template <typename LatticeT>
df_base<LatticeT>::df_base(gw_type gw, gw_type Delta, lattice_t lattice, kmesh kgrid):
    lattice_(lattice),
    fgrid_(gw.grid()),
    kgrid_(kgrid),
    gw_(gw),
    delta_(Delta),
    disp_(gftools::tuple_tools::repeater<kmesh,NDim>::get_tuple(kgrid_)), 
    gd0_(std::tuple_cat(std::forward_as_tuple(fgrid_), gftools::tuple_tools::repeater<kmesh,NDim>::get_array(kgrid_))), 
    gd_(gd0_.grids()),
    sigma_d_(gd0_.grids()),
    glat_(gd0_.grids())
{
    disp_.fill(lattice_.get_dispersion());
    glat_ = this->glat_dmft();
    for (auto w : fgrid_.points()) { gd0_[w] = glat_[w] - gw_[w]; }
    gd_ = gd0_; 
    sigma_d_ = 0.0;
}
    
template <typename LatticeT>
typename df_base<LatticeT>::gk_type df_base<LatticeT>::glat_dmft() const
{
    gk_type glat_dmft(gd0_.grids());
    for (auto w : fgrid_.points()) { 
        glat_dmft[w] = 1.0 / ( 1.0 / gw_[w] + delta_[w] - disp_.data());  
        //if (!is_float_equal(glat_dmft[w].sum()/pow<D>(kpts), gw[w], 1e-3)) ERROR(glat_dmft[w].sum()/pow<D>(kpts) << " != " << gw[w]);
        assert(is_float_equal(glat_dmft[w].sum()/pow<NDim>(kgrid_.size()), gw_[w], 1e-3)); // check consistency of gw, delta and lattice gk
        }
    return glat_dmft;
}

template <typename LatticeT>
typename df_base<LatticeT>::gw_type df_base<LatticeT>::sigma_dmft(double mu) const
{
    gw_type sigma_out(fgrid_);
    for (auto iw : fgrid_.points()) { 
        sigma_out[iw] =  iw.value() + mu - delta_[iw] - 1./gw_[iw];
        };
    return sigma_out;
}

template <typename LatticeT>
typename df_base<LatticeT>::gk_type df_base<LatticeT>::sigma_lat(double mu) const
{
    gk_type sigma_lat(this->gd0_.grids());
    sigma_lat = 0;
    double non_zero = !is_float_equal(sigma_d_.diff(sigma_lat), 0, 1e-12);
    gw_type sigma_dmft1 = this->sigma_dmft(mu);
    for (auto w : fgrid_.points()) { 
        sigma_lat[w] = sigma_dmft1[w] + non_zero / ( 1.0 + sigma_d_[w] * gw_[w] ) * sigma_d_[w];
        }
    return sigma_lat;
}

template <typename LatticeT>
typename df_base<LatticeT>::gw_type df_base<LatticeT>::glat_loc() const
{
    gw_type glatloc(fgrid_);
    double knorm = boost::math::pow<NDim>(kgrid_.size());
    for (auto w : fgrid_.points()) { 
        glatloc[w] = glat_[w].sum()/knorm; 
        }
    return std::move(glatloc);
}

template class df_base<cubic_traits<2>>;
template class df_base<cubic_traits<3>>;

} // end of namespace open_df