declare module 'erlpack' {
	export function pack(data: unknown): Buffer;
	export function unpack<T = unknown>(data: Buffer): T; 
}
