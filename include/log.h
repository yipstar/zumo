//
// Created by yipstar on 2019-03-02.
//

#ifndef ZUMOSM_LOG_H
#define ZUMOSM_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RC_DEBUG
 #define RC_PRINT(x)  Serial.print(x)
 #define RC_PRINTLN(x) Serial.println(x)
#else
 #define RC_PRINT(x)
 #define RC_PRINTLN(x)
#endif

#ifdef __cplusplus
}
#endif

#endif //ZUMOSM_APP_H