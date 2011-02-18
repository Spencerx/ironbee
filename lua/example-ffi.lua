----------------------------------------------------------------------------
-- Define local versions of globals that will be used
----------------------------------------------------------------------------
local base = _G
local ibffi = require("ironbee-ffi")

----------------------------------------------------------------------------
-- Declare the rest of the file as a module and register the module
-- table with ironbee.
----------------------------------------------------------------------------
module(...)
ibffi.register_module(_M)


----------------------------------------------------------------------------
-- Setup some module metadata
----------------------------------------------------------------------------
_COPYRIGHT = "Copyright (C) 2010-2011 Qualys, Inc."
_DESCRIPTION = "IronBee example Lua module"
_VERSION = "0.1"

----------------------------------------------------------------------------
-- Event Handlers
--
-- NOTE: As a best practice, you should avoid using the "onEvent" prefix
-- in any public functions that are NOT to be used as event handlers as
-- these may be treated specially by the engine.
----------------------------------------------------------------------------

-- This is called when the request headers are avalable to inspect.
function onEventHandleRequestHeaders(ib, tx)
    local c_tx = ibffi.cast_tx(tx);

    ibffi.ib_log_debug(ib, 4, "%s.onEventHandleRequestHeaders ib=%p tx=%p",
                       _NAME, ib, tx)
 
    local req_line = ibffi.ib_data_get(c_tx.dpi, "request_line")
    ibffi.ib_log_debug(ib, 4, "Request Line: %s", req_line);

    -- Do something interesting

    return 0
end

-- This is called when there is incoming data for the transaction
function onEventTxDataIn(ib, txdata)
    local c_txdata = ibffi.cast_txdata(txdata);

    -- Just dump the data to the logs
    ibffi.ib_log_debug(ib, 4, "%s.onEventTxDataIn[%d]: %.*s",
                       _NAME, c_txdata.dtype, c_txdata.dlen, c_txdata.data)
 
    return 0
end

