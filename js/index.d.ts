declare module 'erlpack' {
	export function pack(data: any): Buffer;
	export function unpack<T = any>(data: Buffer): T; 
}
