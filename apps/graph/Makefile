apps += Graph::App
app_headers += apps/graph/app.h

app_src += $(addprefix apps/graph/,\
  app.cpp \
  storage_cartesian_function_store.cpp \
  graph/banner_view.cpp \
  graph/calculation_graph_controller.cpp \
  graph/calculation_parameter_controller.cpp \
  graph/curve_parameter_controller.cpp \
  graph/extremum_graph_controller.cpp \
  graph/graph_controller.cpp \
  graph/graph_controller_helper.cpp \
  graph/graph_view.cpp \
  graph/integral_graph_controller.cpp \
  graph/intersection_graph_controller.cpp \
  graph/root_graph_controller.cpp \
  graph/tangent_graph_controller.cpp \
  list/list_parameter_controller.cpp \
  list/storage_list_controller.cpp \
  list/text_field_function_title_cell.cpp \
  values/storage_derivative_parameter_controller.cpp \
  values/storage_function_parameter_controller.cpp \
  values/storage_values_controller.cpp \
)

i18n_files += $(addprefix apps/graph/,\
  base.de.i18n\
  base.en.i18n\
  base.es.i18n\
  base.fr.i18n\
  base.pt.i18n\
)

$(eval $(call depends_on_image,apps/graph/app.cpp,apps/graph/graph_icon.png))
