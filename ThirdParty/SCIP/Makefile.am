# Copyright (C) 2011 Stefan Vigerske
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

## $Id: Makefile.am 937 2010-10-02 19:05:12Z stefan $

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST =

lib_LTLIBRARIES = libcoinscip.la

libcoinscip_la_SOURCES = \
  scip/src/scip/branch.c \
  scip/src/scip/bitencode.c \
  scip/src/scip/buffer.c \
  scip/src/scip/clock.c \
  scip/src/scip/conflict.c \
  scip/src/scip/cons.c \
  scip/src/scip/cutpool.c \
  scip/src/scip/debug.c \
  scip/src/scip/dialog.c \
  scip/src/scip/disp.c \
  scip/src/scip/event.c \
  scip/src/scip/expression.c \
  scip/src/scip/fileio.c \
  scip/src/scip/heur.c \
  scip/src/scip/history.c \
  scip/src/scip/implics.c \
  scip/src/scip/interrupt.c \
  scip/src/scip/intervalarith.c \
  scip/src/scip/lp.c \
  scip/src/scip/mem.c \
  scip/src/scip/message.c \
  scip/src/scip/misc.c \
  scip/src/scip/nlp.c \
  scip/src/scip/nodesel.c \
  scip/src/scip/paramset.c \
  scip/src/scip/presol.c \
  scip/src/scip/pricestore.c \
  scip/src/scip/pricer.c \
  scip/src/scip/primal.c \
  scip/src/scip/prob.c \
  scip/src/scip/prop.c \
  scip/src/scip/reader.c \
  scip/src/scip/relax.c \
  scip/src/scip/retcode.c \
  scip/src/scip/scip.c \
  scip/src/scip/scipdefplugins.c \
  scip/src/scip/scipshell.c \
  scip/src/scip/sepa.c \
  scip/src/scip/sepastore.c \
  scip/src/scip/set.c \
  scip/src/scip/sol.c \
  scip/src/scip/solve.c \
  scip/src/scip/stat.c \
  scip/src/scip/tree.c \
  scip/src/scip/var.c \
  scip/src/scip/vbc.c \
  scip/src/scip/branch_allfullstrong.c \
  scip/src/scip/branch_fullstrong.c \
  scip/src/scip/branch_inference.c \
  scip/src/scip/branch_mostinf.c \
  scip/src/scip/branch_leastinf.c \
  scip/src/scip/branch_pscost.c \
  scip/src/scip/branch_random.c \
  scip/src/scip/branch_relpscost.c \
  scip/src/scip/cons_and.c \
  scip/src/scip/cons_binpack.c \
  scip/src/scip/cons_bounddisjunction.c \
  scip/src/scip/cons_conjunction.c \
  scip/src/scip/cons_eqknapsack.c \
  scip/src/scip/cons_indicator.c \
  scip/src/scip/cons_integral.c \
  scip/src/scip/cons_invarknapsack.c \
  scip/src/scip/cons_knapsack.c \
  scip/src/scip/cons_linear.c \
  scip/src/scip/cons_logicor.c \
  scip/src/scip/cons_or.c \
  scip/src/scip/cons_setppc.c \
  scip/src/scip/cons_varbound.c \
  scip/src/scip/cons_xor.c \
  scip/src/scip/cons_countsols.c \
  scip/src/scip/cons_sos1.c \
  scip/src/scip/cons_sos2.c \
  scip/src/scip/cons_quadratic.c \
  scip/src/scip/cons_cumulative.c \
  scip/src/scip/cons_linking.c \
  scip/src/scip/cons_orbitope.c \
  scip/src/scip/cons_soc.c \
  scip/src/scip/dialog_default.c \
  scip/src/scip/disp_default.c \
  scip/src/scip/heur_actconsdiving.c \
  scip/src/scip/heur_coefdiving.c \
  scip/src/scip/heur_crossover.c \
  scip/src/scip/heur_feaspump.c \
  scip/src/scip/heur_fixandinfer.c \
  scip/src/scip/heur_fracdiving.c \
  scip/src/scip/heur_guideddiving.c \
  scip/src/scip/heur_intdiving.c \
  scip/src/scip/heur_intshifting.c \
  scip/src/scip/heur_linesearchdiving.c \
  scip/src/scip/heur_localbranching.c \
  scip/src/scip/heur_mutation.c \
  scip/src/scip/heur_objpscostdiving.c \
  scip/src/scip/heur_octane.c \
  scip/src/scip/heur_oneopt.c \
  scip/src/scip/heur_pscostdiving.c \
  scip/src/scip/heur_rens.c \
  scip/src/scip/heur_rins.c \
  scip/src/scip/heur_rootsoldiving.c \
  scip/src/scip/heur_rounding.c \
  scip/src/scip/heur_shifting.c \
  scip/src/scip/heur_simplerounding.c \
  scip/src/scip/heur_veclendiving.c \
  scip/src/scip/heur_dins.c \
  scip/src/scip/heur_trivial.c \
  scip/src/scip/heur_trysol.c \
  scip/src/scip/heur_shiftandpropagate.c \
  scip/src/scip/heur_subnlp.c \
  scip/src/scip/heur_twoopt.c \
  scip/src/scip/heur_undercover.c \
  scip/src/scip/heur_zirounding.c \
  scip/src/scip/nodesel_bfs.c \
  scip/src/scip/nodesel_dfs.c \
  scip/src/scip/nodesel_estimate.c \
  scip/src/scip/nodesel_hybridestim.c \
  scip/src/scip/nodesel_restartdfs.c \
  scip/src/scip/presol_dualfix.c \
  scip/src/scip/presol_implics.c \
  scip/src/scip/presol_inttobinary.c \
  scip/src/scip/presol_probing.c \
  scip/src/scip/presol_trivial.c \
  scip/src/scip/presol_boundshift.c \
  scip/src/scip/prop_pseudoobj.c \
  scip/src/scip/prop_rootredcost.c \
  scip/src/scip/prop_vbounds.c \
  scip/src/scip/reader_cnf.c \
  scip/src/scip/reader_fix.c \
  scip/src/scip/reader_lp.c \
  scip/src/scip/reader_mps.c \
  scip/src/scip/reader_sol.c \
  scip/src/scip/reader_zpl.c \
  scip/src/scip/reader_ccg.c \
  scip/src/scip/reader_cip.c \
  scip/src/scip/reader_opb.c \
  scip/src/scip/reader_ppm.c \
  scip/src/scip/reader_rlp.c \
  scip/src/scip/reader_gms.c \
  scip/src/scip/reader_fzn.c \
  scip/src/scip/reader_pip.c \
  scip/src/scip/reader_wbo.c \
  scip/src/scip/sepa_clique.c \
  scip/src/scip/sepa_cmir.c \
  scip/src/scip/sepa_flowcover.c \
  scip/src/scip/sepa_gomory.c \
  scip/src/scip/sepa_impliedbounds.c \
  scip/src/scip/sepa_intobj.c \
  scip/src/scip/sepa_redcost.c \
  scip/src/scip/sepa_strongcg.c \
  scip/src/scip/sepa_mcf.c \
  scip/src/scip/sepa_zerohalf.c \
  scip/src/scip/sepa_cgmip.c \
  scip/src/scip/sepa_oddcycle.c \
  scip/src/scip/sepa_rapidlearning.c \
  scip/src/tclique/tclique_branch.c \
  scip/src/tclique/tclique_coloring.c \
  scip/src/tclique/tclique_graph.c \
  scip/src/blockmemshell/memory.c \
  scip/src/nlpi/nlpi.c \
  scip/src/nlpi/nlpioracle.c \
  scip/src/nlpi/expression.c \
  scip/src/nlpi/intervalarith.cpp \
  scip/src/dijkstra/dijkstra_bh.c \
  scip/src/xml/xmlparse.c

# add LP solver interface
if SCIPLPI_SPX
  libcoinscip_la_SOURCES += scip/src/scip/lpi_spx.cpp
endif
if SCIPLPI_CLP
  libcoinscip_la_SOURCES += scip/src/scip/lpi_clp.cpp
endif
if SCIPLPI_CPX
  libcoinscip_la_SOURCES += scip/src/scip/lpi_cpx.c
endif
if SCIPLPI_MSK
  libcoinscip_la_SOURCES += scip/src/scip/lpi_msk.c
endif
if SCIPLPI_XPR
  libcoinscip_la_SOURCES += scip/src/scip/lpi_xprs.c
endif
if SCIPLPI_GRB
  libcoinscip_la_SOURCES += scip/src/scip/lpi_grb.c
endif
if SCIPLPI_QSO
  libcoinscip_la_SOURCES += scip/src/scip/lpi_qso.c
endif
if SCIPLPI_NONE
  libcoinscip_la_SOURCES += scip/src/scip/lpi_none.c
endif

# add IPOPT as nonlinear solver, if available
if COIN_HAS_IPOPT
libcoinscip_la_SOURCES += scip/src/nlpi/nlpi_ipopt.cpp
else
libcoinscip_la_SOURCES += scip/src/nlpi/nlpi_ipopt_dummy.c
endif

# use CppAD for expression evaluation, if available
if COIN_HAS_CPPAD
libcoinscip_la_SOURCES += scip/src/nlpi/exprinterpret_cppad.cpp
else
libcoinscip_la_SOURCES += scip/src/nlpi/exprinterpret_none.c
endif

AM_CPPFLAGS = -I`$(CYGPATH_W) $(srcdir)/scip/src` $(SCIPLIB_CFLAGS)

DEFAULT_INCLUDES = -I. -I`$(CYGPATH_W) $(srcdir)`

libcoinscip_la_LDFLAGS = $(LT_LDFLAGS)

thirdpartyscipincludedir = $(includedir)/coin/ThirdParty/scip
thirdpartyscipinclude_HEADERS = \
  scip/src/scip/branch_allfullstrong.h \
  scip/src/scip/branch_fullstrong.h \
  scip/src/scip/branch_inference.h \
  scip/src/scip/branch_leastinf.h \
  scip/src/scip/branch_mostinf.h \
  scip/src/scip/branch_pscost.h \
  scip/src/scip/branch_random.h \
  scip/src/scip/branch_relpscost.h \
  scip/src/scip/buffer.h \
  scip/src/scip/clock.h \
  scip/src/scip/cons_and.h \
  scip/src/scip/cons_binpack.h \
  scip/src/scip/cons_bounddisjunction.h \
  scip/src/scip/cons_conjunction.h \
  scip/src/scip/cons_countsols.h \
  scip/src/scip/cons_cumulative.h \
  scip/src/scip/cons_eqknapsack.h \
  scip/src/scip/cons_indicator.h \
  scip/src/scip/cons_integral.h \
  scip/src/scip/cons_invarknapsack.h \
  scip/src/scip/cons_knapsack.h \
  scip/src/scip/cons_linear.h \
  scip/src/scip/cons_linking.h \
  scip/src/scip/cons_logicor.h \
  scip/src/scip/cons_orbitope.h \
  scip/src/scip/cons_or.h \
  scip/src/scip/cons_quadratic.h \
  scip/src/scip/cons_setppc.h \
  scip/src/scip/cons_soc.h \
  scip/src/scip/cons_sos1.h \
  scip/src/scip/cons_sos2.h \
  scip/src/scip/cons_varbound.h \
  scip/src/scip/cons_xor.h \
  scip/src/scip/def.h \
  scip/src/scip/dialog_default.h \
  scip/src/scip/disp_default.h \
  scip/src/scip/heur_actconsdiving.h \
  scip/src/scip/heur_coefdiving.h \
  scip/src/scip/heur_crossover.h \
  scip/src/scip/heur_dins.h \
  scip/src/scip/heur_feaspump.h \
  scip/src/scip/heur_fixandinfer.h \
  scip/src/scip/heur_fracdiving.h \
  scip/src/scip/heur_guideddiving.h \
  scip/src/scip/heur_intdiving.h \
  scip/src/scip/heur_intshifting.h \
  scip/src/scip/heur_linesearchdiving.h \
  scip/src/scip/heur_localbranching.h \
  scip/src/scip/heur_mutation.h \
  scip/src/scip/heur_objpscostdiving.h \
  scip/src/scip/heur_octane.h \
  scip/src/scip/heur_oneopt.h \
  scip/src/scip/heur_pscostdiving.h \
  scip/src/scip/heur_rens.h \
  scip/src/scip/heur_rins.h \
  scip/src/scip/heur_rootsoldiving.h \
  scip/src/scip/heur_rounding.h \
  scip/src/scip/heur_shiftandpropagate.h \
  scip/src/scip/heur_shifting.h \
  scip/src/scip/heur_simplerounding.h \
  scip/src/scip/heur_subnlp.h \
  scip/src/scip/heur_trivial.h \
  scip/src/scip/heur_trysol.h \
  scip/src/scip/heur_twoopt.h \
  scip/src/scip/heur_undercover.h \
  scip/src/scip/heur_veclendiving.h \
  scip/src/scip/heur_zirounding.h \
  scip/src/scip/history.h \
  scip/src/scip/implics.h \
  scip/src/scip/intervalarith.h \
  scip/src/scip/lpi.h \
  scip/src/scip/message.h \
  scip/src/scip/misc.h \
  scip/src/scip/nodesel_bfs.h \
  scip/src/scip/nodesel_dfs.h \
  scip/src/scip/nodesel_estimate.h \
  scip/src/scip/nodesel_hybridestim.h \
  scip/src/scip/nodesel_restartdfs.h \
  scip/src/scip/presol_boundshift.h \
  scip/src/scip/presol_dualfix.h \
  scip/src/scip/presol_implics.h \
  scip/src/scip/presol_inttobinary.h \
  scip/src/scip/presol_probing.h \
  scip/src/scip/presol_trivial.h \
  scip/src/scip/prop_pseudoobj.h \
  scip/src/scip/prop_rootredcost.h \
  scip/src/scip/prop_vbounds.h \
  scip/src/scip/pub_branch.h \
  scip/src/scip/pub_conflict.h \
  scip/src/scip/pub_cons.h \
  scip/src/scip/pub_cutpool.h \
  scip/src/scip/pub_dialog.h \
  scip/src/scip/pub_disp.h \
  scip/src/scip/pub_event.h \
  scip/src/scip/pub_expression.h \
  scip/src/scip/pub_fileio.h \
  scip/src/scip/pub_heur.h \
  scip/src/scip/pub_implics.h \
  scip/src/scip/pub_lp.h \
  scip/src/scip/pub_message.h \
  scip/src/scip/pub_misc.h \
  scip/src/scip/pub_nlp.h \
  scip/src/scip/pub_nodesel.h \
  scip/src/scip/pub_paramset.h \
  scip/src/scip/pub_presol.h \
  scip/src/scip/pub_pricer.h \
  scip/src/scip/pub_prop.h \
  scip/src/scip/pub_reader.h \
  scip/src/scip/pub_relax.h \
  scip/src/scip/pub_sepa.h \
  scip/src/scip/pub_sol.h \
  scip/src/scip/pub_tree.h \
  scip/src/scip/pub_var.h \
  scip/src/scip/reader_ccg.h \
  scip/src/scip/reader_cip.h \
  scip/src/scip/reader_cnf.h \
  scip/src/scip/reader_fix.h \
  scip/src/scip/reader_fzn.h \
  scip/src/scip/reader_gms.h \
  scip/src/scip/reader_lp.h \
  scip/src/scip/reader_mps.h \
  scip/src/scip/reader_opb.h \
  scip/src/scip/reader_pip.h \
  scip/src/scip/reader_ppm.h \
  scip/src/scip/reader_rlp.h \
  scip/src/scip/reader_sol.h \
  scip/src/scip/reader_wbo.h \
  scip/src/scip/reader_zpl.h \
  scip/src/scip/scipdefplugins.h \
  scip/src/scip/scip.h \
  scip/src/scip/scipshell.h \
  scip/src/scip/sepa_cgmip.h \
  scip/src/scip/sepa_clique.h \
  scip/src/scip/sepa_cmir.h \
  scip/src/scip/sepa_flowcover.h \
  scip/src/scip/sepa_gomory.h \
  scip/src/scip/sepa_impliedbounds.h \
  scip/src/scip/sepa_intobj.h \
  scip/src/scip/sepa_mcf.h \
  scip/src/scip/sepa_oddcycle.h \
  scip/src/scip/sepa_rapidlearning.h \
  scip/src/scip/sepa_redcost.h \
  scip/src/scip/sepa_strongcg.h \
  scip/src/scip/sepa_zerohalf.h \
  scip/src/scip/set.h \
  scip/src/scip/struct_branch.h \
  scip/src/scip/struct_buffer.h \
  scip/src/scip/struct_clock.h \
  scip/src/scip/struct_conflict.h \
  scip/src/scip/struct_cons.h \
  scip/src/scip/struct_cutpool.h \
  scip/src/scip/struct_dialog.h \
  scip/src/scip/struct_disp.h \
  scip/src/scip/struct_event.h \
  scip/src/scip/struct_heur.h \
  scip/src/scip/struct_history.h \
  scip/src/scip/struct_implics.h \
  scip/src/scip/struct_lp.h \
  scip/src/scip/struct_mem.h \
  scip/src/scip/struct_message.h \
  scip/src/scip/struct_misc.h \
  scip/src/scip/struct_nlp.h \
  scip/src/scip/struct_nodesel.h \
  scip/src/scip/struct_paramset.h \
  scip/src/scip/struct_presol.h \
  scip/src/scip/struct_pricer.h \
  scip/src/scip/struct_pricestore.h \
  scip/src/scip/struct_primal.h \
  scip/src/scip/struct_prob.h \
  scip/src/scip/struct_prop.h \
  scip/src/scip/struct_reader.h \
  scip/src/scip/struct_relax.h \
  scip/src/scip/struct_scip.h \
  scip/src/scip/struct_sepa.h \
  scip/src/scip/struct_sepastore.h \
  scip/src/scip/struct_set.h \
  scip/src/scip/struct_sol.h \
  scip/src/scip/struct_stat.h \
  scip/src/scip/struct_tree.h \
  scip/src/scip/struct_var.h \
  scip/src/scip/struct_vbc.h \
  scip/src/scip/tree.h \
  scip/src/scip/type_branch.h \
  scip/src/scip/type_buffer.h \
  scip/src/scip/type_clock.h \
  scip/src/scip/type_conflict.h \
  scip/src/scip/type_cons.h \
  scip/src/scip/type_cutpool.h \
  scip/src/scip/type_dialog.h \
  scip/src/scip/type_disp.h \
  scip/src/scip/type_event.h \
  scip/src/scip/type_heur.h \
  scip/src/scip/type_history.h \
  scip/src/scip/type_implics.h \
  scip/src/scip/type_interrupt.h \
  scip/src/scip/type_lp.h \
  scip/src/scip/type_lpi.h \
  scip/src/scip/type_mem.h \
  scip/src/scip/type_message.h \
  scip/src/scip/type_misc.h \
  scip/src/scip/type_nlp.h \
  scip/src/scip/type_nodesel.h \
  scip/src/scip/type_paramset.h \
  scip/src/scip/type_presol.h \
  scip/src/scip/type_pricer.h \
  scip/src/scip/type_pricestore.h \
  scip/src/scip/type_primal.h \
  scip/src/scip/type_prob.h \
  scip/src/scip/type_prop.h \
  scip/src/scip/type_reader.h \
  scip/src/scip/type_relax.h \
  scip/src/scip/type_result.h \
  scip/src/scip/type_retcode.h \
  scip/src/scip/type_scip.h \
  scip/src/scip/type_sepa.h \
  scip/src/scip/type_sepastore.h \
  scip/src/scip/type_set.h \
  scip/src/scip/type_sol.h \
  scip/src/scip/type_stat.h \
  scip/src/scip/type_tree.h \
  scip/src/scip/type_var.h \
  scip/src/scip/type_vbc.h

thirdpartybmsincludedir = $(includedir)/coin/ThirdParty/blockmemshell
thirdpartybmsinclude_HEADERS = scip/src/blockmemshell/memory.h

thirdpartynlpiincludedir = $(includedir)/coin/ThirdParty/nlpi
thirdpartynlpiinclude_HEADERS = \
  scip/src/nlpi/exprinterpret.h \
  scip/src/nlpi/nlpi_ipopt.h \
  scip/src/nlpi/pub_expression.h \
  scip/src/nlpi/struct_expression.h \
  scip/src/nlpi/struct_nlpi.h \
  scip/src/nlpi/type_expression.h \
  scip/src/nlpi/type_exprinterpret.h \
  scip/src/nlpi/type_nlpi.h

pkgconfiglibdir = $(libdir)/pkgconfig
pkgconfiglib_DATA = coinscip.pc

test:
	@echo "No test available for SCIP."

########################################################################
#                         Maintainer Stuff                             #
########################################################################

# Files that are generated and should be cleaned with make distclean
DISTCLEANFILES =

include BuildTools/Makemain.inc
