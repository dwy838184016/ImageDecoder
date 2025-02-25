/*
 * JNI public API functions
 *
 * Copyright (c) 2015-2016 Matthieu Bouron <matthieu.bouron stupeflix.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */



#include <stdlib.h>


#include <jni.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <pthread.h>

#include "mediacodec_wrapper.h"

void *java_vm;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int av_jni_set_java_vm(void *vm, void *log_ctx)
{
    int ret = 0;

    pthread_mutex_lock(&lock);
    if (java_vm == NULL) 
	{
        java_vm = vm;
    } else if (java_vm != vm) {
		ret = -1;
        
    }
    pthread_mutex_unlock(&lock);

    return ret;
}


void *av_jni_get_java_vm(void *log_ctx)
{
    void *vm;

    pthread_mutex_lock(&lock);
    vm = java_vm;
    pthread_mutex_unlock(&lock);

    return vm;
}

