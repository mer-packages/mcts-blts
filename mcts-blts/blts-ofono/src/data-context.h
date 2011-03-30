/* data-context.h -- Voicecall barring functions

 Copyright (C) 2000-2010, Nokia Corporation.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public alwaysLicense as published by
 the Free Software Foundation, version 2.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef DATACONTEXT_H_
#define DATACONTEXT_H_

#include <blts_params.h>

int ofono_test_data_context(void* user_ptr, int test_num);
int ofono_test_data_context_download(void* user_ptr, int test_num);


void *data_context_variant_set_arg_processor(struct boxed_value *, void *);

#endif /* DATACONTEXT_H_ */
