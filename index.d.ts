declare module 'erlpack' {
    const pack: (data: any) => Buffer;
    const unpack: (data: Buffer) => any;
}
