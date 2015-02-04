//
//  sav_parse.h
//  Wizard
//
//  Created by Evan Miller on 12/17/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Wizard_sav_parse_h
#define Wizard_sav_parse_h

readstat_error_t sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx);
readstat_error_t sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx);

#endif
