import { handleMessage } from './ipc/message-handler';
import { MessageToPlugin } from './types/message-to-plugin';

/**
 * SendParameterValueFromUI
 */
export function SPVFUI(_paramIdx: number, _value: number) {
  // no-op
}

/**
 * BeginInformHostOfParamChangeFromUI
 */
export function BPCFUI(_paramIdx: number) {
  // no-op
}

/**
 * EndInformHostOfParamChangeFromUI
 */
export function EPCFUI(_paramIdx: number) {
  // no-op
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
export function SAMFD(msgTag: number, _dataSize: number, msg: string) {
  handleMessage(msgTag, atob(msg));
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
export function OnParamChange(_paramIdx: number, _value: number) {
  // if (SPVFD) {
  //   SPVFD(paramIdx, value);
  // }
}

// We want to be able to run the frontend in a browser for testing purposes.
// To keep from runtime errors, add this function to the window object if necessary.
if (window.IPlugSendMsg === undefined) {
  window.IPlugSendMsg = <T>(_msg: MessageToPlugin<T>) => {
    // no-op
  };
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
