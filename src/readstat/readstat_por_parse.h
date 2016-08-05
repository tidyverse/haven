//
//  readstat_por_parse.h
//  Wizard
//
//  Created by Evan Miller on 4/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Wizard_readstat_por_parse_h
#define Wizard_readstat_por_parse_h

int readstat_por_parse_double(const char *data, size_t len, double *result, readstat_error_handler error_cb, void *user_ctx);

#endif
