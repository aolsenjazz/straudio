import { KnownIncomingMessage } from './signal-messages';

function isObject(obj: unknown): obj is Record<string, unknown> {
  return typeof obj === 'object' && obj !== null;
}

export function isKnownIncomingMessage(
  msg: unknown
): msg is KnownIncomingMessage {
  if (!isObject(msg)) return false;
  if (typeof msg.type !== 'string') return false;
  if (typeof msg.timeSent !== 'number') return false;

  switch (msg.type) {
    case 'joinedSession': {
      if (!isObject(msg.payload)) return false;
      const p = msg.payload as Record<string, unknown>;
      return (
        typeof p.sessionId === 'string' &&
        typeof p.hostId === 'string' &&
        (p.role === undefined || p.role === 'guest' || p.role === 'host')
      );
    }

    case 'offer':
    case 'answer': {
      // Incoming offers/answers should have a payload with sourceClientId and description
      if (!isObject(msg.payload)) return false;
      const p = msg.payload as Record<string, unknown>;
      return (
        typeof p.sourceClientId === 'string' &&
        typeof p.description === 'string'
      );
    }

    case 'candidate': {
      // Incoming candidate messages should have a payload with sourceClientId, candidate, and mid
      if (!isObject(msg.payload)) return false;
      const p = msg.payload as Record<string, unknown>;
      return (
        typeof p.sourceClientId === 'string' &&
        typeof p.candidate === 'string' &&
        typeof p.mid === 'string'
      );
    }

    case 'ping':
    case 'pong':
      return true;

    default:
      return false;
  }
}
