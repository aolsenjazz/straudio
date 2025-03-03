/**
 * SendParameterValueFromUI
 */
export function SPVFUI(paramIdx: number, value: number) {
  if (paramIdx < 0) {
    console.log('SPVFUI paramIdx must be >= 0');
    return;
  }

  const message = {
    msg: 'SPVFUI',
    paramIdx: parseInt(String(paramIdx)),
    value: value,
  };

  IPlugSendMsg(message);
}

/**
 * BeginInformHostOfParamChangeFromUI
 */
export function BPCFUI(paramIdx: number) {
  if (paramIdx < 0) {
    console.log('BPCFUI paramIdx must be >= 0');
    return;
  }

  const message = {
    msg: 'BPCFUI',
    paramIdx: parseInt(String(paramIdx)),
  };

  IPlugSendMsg(message);
}

/**
 * EndInformHostOfParamChangeFromUI
 */
export function EPCFUI(paramIdx: number) {
  if (paramIdx < 0) {
    console.log('EPCFUI paramIdx must be >= 0');
    return;
  }

  const message = {
    msg: 'EPCFUI',
    paramIdx: parseInt(String(paramIdx)),
  };

  IPlugSendMsg(message);
}

/**
 * SendParameterValueFromDelegate: receive parameter values(?) from the plugin here.
 */
export function SPVFD(paramIdx: number, val: number) {
  OnParamChange(paramIdx, val);
}

//
/**
 * SendArbitraryMsgFromDelegate: receive messages from the plugin here.
 *
 * @param msgTag Unknown
 * @param dataSize Size of data in bytes
 * @param msg Base64 encoded data. Decode with window.atob()
 */
export function SAMFD(_msgTag: number, _dataSize: number, _msg: string) {
  // no-op
}

/**
 * SendControlValueFromDelegate
 *
 * Purpose unclear. Controlchange?
 */
export function SCVFD(_ctrlTag: number, _val: number) {
  // no-op
}

/**
 * SendControlMsgFromDelegate
 *
 * Purpose unclear. Controlchange messages?
 */
export function SCMFD(
  _ctrlTag: number,
  _msgTag: number,
  _dataSize: number,
  _msg: string
) {
  // no-op
}

/**
 * ?
 */
export function OnParamChange(paramIdx: number, value: number) {
  if (SPVFD) {
    SPVFD(paramIdx, value);
  }
}

// Make these functions globally available
globalThis.SPVFUI = SPVFUI;
globalThis.BPCFUI = BPCFUI;
globalThis.EPCFUI = EPCFUI;
globalThis.SAMFD = SAMFD;
globalThis.SPVFD = SPVFD;
globalThis.SCVFD = SCVFD;
globalThis.SCMFD = SCMFD;
globalThis.OnParamChange = OnParamChange;
