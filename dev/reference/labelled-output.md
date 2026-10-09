# Labelled vector output

Labelled vector output

## Value

A tibble, data frame variant with nice defaults.

Variable labels are stored in the "label" attribute of each variable. It
is not printed on the console, but the RStudio viewer will show it.

Value labels are preserved with the
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
class. Labelled vectors are an intermediate representation that
preserves the original value labels, not a regular R factor. Convert
labelled categorical variables with
[`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md),
or remove value labels with
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
if you need plain R vectors for analysis. See
[`vignette("semantics")`](https://haven.tidyverse.org/dev/articles/semantics.md)
for more details.
