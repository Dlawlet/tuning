export const EOF = 0;
export const NUMBER = 1;
export const TOKEN0 = 2;
export const TOKEN1 = 3;
export const TOKEN2 = 4;

export class CmdLineScanner {
    constructor(cmdline) {
        this.cmdline = cmdline;
        this.pos = 0;
        this.scan();
    }
    peek() {
        return this.lookahead;
    }
    next() {
        const nextToken = this.lookahead;
        this.scan();
        return nextToken;
    }
    scan() {
        let start = this.pos;
        while (start < this.cmdline.length && this.cmdline[start] === " ")
            start++;
        let end = start;
        while (end < this.cmdline.length && this.cmdline[end] !== " ")
            end++;
        this.pos = end;
        if (start !== end)
            this.lookahead = this.makeToken(this.cmdline.substring(start, end));
        else
            this.lookahead = { type: EOF };
    }
    makeToken(str) {
        let n = Number(str);
        if (!isNaN(n)) {
            return { type: NUMBER, value: Number(n) };
        }
        else {
            // handle tokens that begin with --
            if (this.cmdline[0] !== "-" || this.cmdline[1] !== "-")
                throw "Invalid token found";
            return this.makeNonDigitToken(str);
        }
    }
    makeNonDigitToken(str) {
        let length = str.length;
        // check if the token contains digits
        if (this.isDigit(str[length - 1])) {
            if (str[length - 2] === "x" && this.isDigit(str[length - 3])) {
                // --name$x$ token
                return {
                    type: TOKEN2,
                    value: str.substring(2, length - 3),
                    digit1: Number(str[length - 3]),
                    digit2: Number(str[length - 1])
                };
            }
            else {
                // --name$ token
                return {
                    type: TOKEN1,
                    value: str.substring(2, length - 1),
                    digit1: Number(str[length - 1])
                };
            }
        }
        else {
            // --name token
            return { type: TOKEN0, value: str.substring(2, length) };
        }
    }
    isDigit(str) {
        return str.length === 1 && str >= "0" && str <= "9";
    }
}
