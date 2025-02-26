type WebSocketCallback = (data: any) => void;

export class WebSocketService {
  private ws: WebSocket | null = null;
  private subscribers: Map<string, WebSocketCallback[]> = new Map();

  constructor(private url: string) {}

  connect() {
    const token = localStorage.getItem('token');
    this.ws = new WebSocket(`${this.url}?token=${token}`);
    // this.ws = new WebSocket(this.url);

    this.ws.onopen = () => {
      console.log('WebSocket Connected');
    };

    this.ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      const { type, payload } = data;
      
      if (this.subscribers.has(type)) {
        this.subscribers.get(type)?.forEach(callback => callback(payload));
      }
    };

    this.ws.onerror = (error) => {
      console.error('WebSocket Error:', error);
    };

    this.ws.onclose = () => {
      console.log('WebSocket Disconnected');
      // Attempt to reconnect after 5 seconds
      setTimeout(() => this.connect(), 5000);
    };
  }

  subscribe(type: string, callback: WebSocketCallback) {
    if (!this.subscribers.has(type)) {
      this.subscribers.set(type, []);
    }
    this.subscribers.get(type)?.push(callback);
  }

  unsubscribe(type: string, callback: WebSocketCallback) {
    if (this.subscribers.has(type)) {
      const callbacks = this.subscribers.get(type) || [];
      this.subscribers.set(type, callbacks.filter(cb => cb !== callback));
    }
  }

  send(message: object) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    }
  }
}