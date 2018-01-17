# nocov start
.onUnload <- function(libpath) {
  library.dynam.unload("haven", libpath)
}
# nocov end
