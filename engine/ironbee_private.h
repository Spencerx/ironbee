/*****************************************************************************
 * Licensed to Qualys, Inc. (QUALYS) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * QUALYS licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef _IB_PRIVATE_H_
#define _IB_PRIVATE_H_

/**
 * @file
 * @brief IronBee &mdash; Private Declarations
 *
 * @author Brian Rectanus <brectanus@qualys.com>
 */

#include <ironbee/types.h>
#include <ironbee/engine.h>
#include <ironbee/util.h>
#include <ironbee/debug.h>
#include <ironbee/lock.h>
#include <ironbee/server.h>
#include <ironbee/module.h>
#include <ironbee/provider.h>
#include <ironbee/array.h>
#include <ironbee/logformat.h>
#include <ironbee/rule_defs.h>

/* Pull in FILE* for ib_auditlog_cfg_t. */
#include <stdio.h>

/**
 * @internal
 *
 * Internal hook structure
 */
typedef struct ib_hook_t ib_hook_t;
struct ib_hook_t {
    union {
        /*! Comparison only */
        ib_void_fn_t                as_void;

        ib_state_null_hook_fn_t     null;
        ib_state_conn_hook_fn_t     conn;
        ib_state_conndata_hook_fn_t conndata;
        ib_state_tx_hook_fn_t       tx;
        ib_state_txdata_hook_fn_t   txdata;
        ib_state_header_data_fn_t   headerdata;
        ib_state_request_line_fn_t  requestline;
        ib_state_response_line_fn_t responseline;
    } callback;
    void               *cdata;            /**< Data passed to the callback */
    ib_hook_t          *next;             /**< The next callback in the list */
};

/**
 * @internal
 *
 * Rule engine per-context data
 */
typedef struct ib_rule_engine_t ib_rule_engine_t;

/**
 * @internal
 *
 * Engine handle.
 */
struct ib_engine_t {
    ib_mpool_t         *mp;               /**< Primary memory pool */
    ib_mpool_t         *config_mp;        /**< Config memory pool */
    ib_mpool_t         *temp_mp;          /**< Temp memory pool for config */
    ib_provider_inst_t *dpi;              /**< Data provider instance */
    ib_context_t       *ectx;             /**< Engine configuration context */
    ib_context_t       *ctx;              /**< Main configuration context */
    ib_uuid_t           sensor_id;        /**< Sensor UUID */
    uint32_t            sensor_id_hash;   /**< Sensor UUID hash (4 bytes) */
    const char         *sensor_id_str;    /**< ascii format, for logging */
    const char         *sensor_name;      /**< Sensor name */
    const char         *sensor_version;   /**< Sensor version string */
    const char         *sensor_hostname;  /**< Sensor hostname */

    /// @todo Only these should be private
    ib_server_t        *plugin;           /**< Info about the server plugin */
    ib_array_t         *modules;          /**< Array tracking modules */
    ib_array_t         *filters;          /**< Array tracking filters */
    ib_array_t         *contexts;         /**< Configuration contexts */
    ib_hash_t          *dirmap;           /**< Hash tracking directive map */
    ib_hash_t          *apis;             /**< Hash tracking provider APIs */
    ib_hash_t          *providers;        /**< Hash tracking providers */
    ib_hash_t          *tfns;             /**< Hash tracking transformations */
    ib_hash_t          *operators;        /**< Hash tracking operators */
    ib_hash_t          *actions;          /**< Hash tracking rules */
    ib_rule_engine_t   *rules;            /**< Rule engine data */

    /* Hooks */
    ib_hook_t *hook[IB_STATE_EVENT_NUM + 1]; /**< Registered hook callbacks */
};

/**
 * @internal
 *
 * Configuration context data.
 */
typedef struct ib_context_data_t ib_context_data_t;
struct ib_context_data_t {
    ib_module_t        *module;           /**< Module handle */
    void               *data;             /**< Module config structure */
};

/**
 * Per-context audit log configuration.
 *
 * This struct is associated with an owning context by the ib_context_t*
 * member named "owner."
 * Only the owner context may destroy or edit the logging context.
 * Child contexts that copy from the parent context may have a copy of
 * the pointer to this struct, but may not edit its context.
 *
 * Child contexts may, though, lock the index_fp_lock field and write to
 * the index_fp.
 *
 * The owning context should lock index_fp_lock before updating lock_fp and
 * index.
 */
typedef struct ib_auditlog_cfg_t ib_auditlog_cfg_t;

//! See typedef for more details.
struct ib_auditlog_cfg_t {
    char *index;            /**< Index file. */
    FILE *index_fp;         /**< Index file pointer. */
    ib_lock_t index_fp_lock; /**< Lock to protect index_fp. */
    ib_context_t *owner;    /**< Owning context. Only owner should edit. */
};

/**
 * @internal
 *
 * Configuration context.
 */
struct ib_context_t {
    ib_engine_t             *ib;          /**< Engine */
    ib_mpool_t              *mp;          /**< Memory pool */
    ib_cfgmap_t             *cfg;         /**< Config map */
    ib_array_t              *cfgdata;     /**< Config data */
    ib_context_t            *parent;      /**< Parent context */
    const char              *ctx_type;    /**< Type identifier string. */
    const char              *ctx_name;    /**< Name identifier string. */
    const char              *ctx_full;    /**< Full name of context */
    ib_auditlog_cfg_t       *auditlog;    /**< Per-context audit log cfgs. */

    /* Context Selection */
    ib_context_fn_t          fn_ctx;      /**< Context decision function */
    ib_context_site_fn_t     fn_ctx_site; /**< Context site function */
    void                    *fn_ctx_data; /**< Context function data */

    /* Filters */
    ib_list_t               *filters;     /**< Context enabled filters */

    /* Rules associated with this context */
    ib_rule_engine_t        *rules;       /**< Rule engine data */
};

/**
 * @internal
 *
 * Matcher.
 */
struct ib_matcher_t {
    ib_engine_t             *ib;          /**< Engine */
    ib_mpool_t              *mp;          /**< Memory pool */
    ib_provider_t           *mpr;         /**< Matcher provider */
    ib_provider_inst_t      *mpi;         /**< Matcher provider instance */
    const char              *key;         /**< Matcher key */
};

/**
 * Rule execution results for loggging.
 * @internal
 */
struct ib_rule_target_result_t {
    ib_rule_target_t  *target;      /**< Target of rule */
    ib_field_t        *original;    /**< Original value */
    ib_field_t        *transformed; /**< Transformed value */
    ib_num_t           result;      /**< Result of target operation. */
};
typedef struct ib_rule_target_result_t ib_rule_target_result_t;

/**
 * Parameters used for variable expansion in rules.
 */
#define IB_VARIABLE_EXPANSION_PREFIX  "%{"  /**< Variable prefix */
#define IB_VARIABLE_EXPANSION_POSTFIX "}"   /**< Variable postfix */

/**
 * Initialize the core fields.
 *
 * Called when the core is loaded, registers the core field generators.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 */
ib_status_t ib_core_fields_init(ib_engine_t *ib,
                                ib_module_t *mod);

/**
 * Initialize the core config context for fields.
 *
 * Called when the core is loaded, registers the core field generators.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 */
ib_status_t ib_core_fields_ctx_init(ib_engine_t *ib,
                                    ib_module_t *mod,
                                    ib_context_t *ctx,
                                    void *cbdata);

/**
 * @internal
 * Initialize the rule engine.
 *
 * Called when the rule engine is loaded, registers event handlers.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 */
ib_status_t ib_rule_engine_init(ib_engine_t *ib,
                                ib_module_t *mod);

/**
 * @internal
 * Initialize a context the rule engine.
 *
 * Called when a context is initialized, performs rule engine initialization.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 * @param[in,out] ctx IronBee context
 */
ib_status_t ib_rule_engine_ctx_init(ib_engine_t *ib,
                                    ib_module_t *mod,
                                    ib_context_t *ctx);

/**
 * @internal
 * Initialize the core transformations.
 *
 * Called when the rule engine is loaded, registers the core transformations.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 */
ib_status_t ib_core_transformations_init(ib_engine_t *ib,
                                         ib_module_t *mod);

/**
 * @internal
 * Initialize the core operators.
 *
 * Called when the rule engine is loaded, registers the core operators.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 */
ib_status_t ib_core_operators_init(ib_engine_t *ib,
                                   ib_module_t *mod);

/**
 * @internal
 * Initialize the core actions.
 *
 * Called when the rule engine is loaded, registers the core actions.
 *
 * @param[in,out] ib IronBee object
 * @param[in] mod Module object
 */
ib_status_t ib_core_actions_init(ib_engine_t *ib,
                                 ib_module_t *mod);


/**
 * Check that @a event is appropriate for @a hook_type.
 *
 * @param[in] ib IronBee Engine.
 * @param[in] event The event type.
 * @param[in] hook_type The hook that is proposed to match the @a event.
 * @returns IB_OK or IB_EINVAL if @a event is not suitable for @a hook_type.
 */
ib_status_t ib_check_hook(ib_engine_t* ib,
                          ib_state_event_type_t event,
                          ib_state_hook_type_t hook_type);

/**
 * Return the configured rule logging level.
 *
 * This is used to determine if optional complex processing should be
 * performed to log possibly option information.
 *
 * @param[in] ib The IronBee engine that would be used in a call to ib_log_ex.
 * @return The log level configured.
 */
ib_rule_log_level_t ib_rule_log_level(ib_engine_t *ib);

/**
 * Generic Logger for rules.
 *
 * @warning There is currently a 1024 byte formatter limit when prefixing the
 *          log header data.
 *
 * @param level Log level
 * @param tx Transaction information
 * @param rule Rule to log
 * @param target Rule target
 * @param prefix String to prefix log header data (or NULL)
 * @param file Filename (or NULL)
 * @param line Line number (or 0)
 * @param fmt Printf-like format string
 * @param ap Argument list
 */
void ib_rule_vlog(ib_rule_log_level_t level,
                  ib_tx_t *tx,
                  const ib_rule_t *rule,
                  const ib_rule_target_t *target,
                  const char *prefix,
                  const char *file,
                  int line,
                  const char *fmt,
                  va_list ap);

/**
 * Generic Logger for rules.
 *
 * @warning There is currently a 1024 byte formatter limit when prefixing the
 *          log header data.
 *
 * @param level Log level
 * @param tx Transaction information
 * @param rule Rule to log
 * @param target Rule target
 * @param prefix String to prefix log header data (or NULL)
 * @param file Filename (or NULL)
 * @param line Line number (or 0)
 * @param fmt Printf-like format string
 */
void ib_rule_log(ib_rule_log_level_t level,
                 ib_tx_t *tx,
                 const ib_rule_t *rule,
                 const ib_rule_target_t *target,
                 const char *prefix,
                 const char *file,
                 int line,
                 const char *fmt, ...)
    PRINTF_ATTRIBUTE(8, 0);

/**
 * "Fast" rule logging
 *
 * @param tx Transaction information
 * @param rule Rule to log
 * @param result_type Log true or false results?
 * @param results List of target results
 * @param actions List of actions executed
 * @param file Source file name
 * @param line Source line number
 */
void ib_rule_log_fast_ex(ib_tx_t *tx,
                         const ib_rule_t *rule,
                         ib_bool_t result_type,
                         const ib_list_t *results,
                         const ib_list_t *actions,
                         const char *file,
                         int line);

/**
 * Log a field's value
 * @internal
 *
 * @param[in] ib Engine
 * @param[in] label Label string
 * @param[in] f Field
 */
void ib_log_rule_field(ib_engine_t *ib,
                       const char *label,
                       const ib_field_t *f);

/** Rule execution logging */
#define ib_rule_log_fast(tx,rule, result_type, results, actions)         \
    ib_rule_log_fast_ex(tx, rule, result_type, results, actions, __FILE__, __LINE__)

/** Rule execution logging */
#define ib_rule_log_exec(tx,rule,target,...) \
    ib_rule_log(IB_RULE_LOG_EXEC, tx, rule, target, "EXECUTION", __FILE__, __LINE__, __VA_ARGS__)

/** Rule full logging */
#define ib_rule_log_full(tx,rule,target,...) \
    ib_rule_log(IB_RULE_LOG_FULL, tx, rule, target, "FULL", __FILE__, __LINE__, __VA_ARGS__)

/** Rule debug logging */
#define ib_rule_log_debug(tx,rule,target,...) \
    ib_rule_log(IB_RULE_LOG_DEBUG, tx, rule, target, "DEBUG", __FILE__, __LINE__, __VA_ARGS__)

/** Rule trace logging */
#define ib_rule_log_trace(tx,rule,target,...) \
    ib_rule_log(IB_RULE_LOG_TRACE, tx, rule, target, "TRACE", __FILE__, __LINE__, __VA_ARGS__)

#endif /* IB_PRIVATE_H_ */
