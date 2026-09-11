// Translated from solution.cpp.

func debug() -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> #d");
}

func getchar() -> dynamic
{
  return cpp_expression("#include <bits/");
}

func putchar(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/s");
}

var IN_BUF: dynamic = (1 << 23);

var OUT_BUF: dynamic = (1 << 23);

func myGetchar() -> dynamic
{
  var buf: dynamic = cpp_array(IN_BUF);
  var ps: dynamic = buf;
  var pt: dynamic = buf;
  if ((ps == pt))
  {
    ps = buf;
    pt = (buf + fread(buf, 1, IN_BUF, stdin));
  }
  return  ((ps == pt)) ? EOF : (*cpp_update(ps, "++"));
}

func read(x: dynamic) -> dynamic
{
  var op: dynamic = 0;
  var ch: dynamic = getchar();
  x = 0;
  {
    while (((!isdigit(ch)) && (ch != EOF)))
    {
      op ^= ((ch == cpp_char("-")));
      ch = getchar();
    }
  }
  if ((ch == EOF))
  {
    return false;
  }
  {
    while (isdigit(ch))
    {
      x = ((x * 10) + ((ch ^ cpp_char("0"))));
      ch = getchar();
    }
  }
  if (op)
  {
    x = (-x);
  }
  return true;
}

func readStr(s: dynamic) -> dynamic
{
  var n: dynamic = 0;
  var ch: dynamic = getchar();
  {
    while ((isspace(ch) && (ch != EOF)))
    {
      ch = getchar();
    }
  }
  {
    while (((!isspace(ch)) && (ch != EOF)))
    {
      s[cpp_update(n, "++")] = ch;
      ch = getchar();
    }
  }
  s[n] = cpp_char("\u{0}");
  return n;
}

func myPutchar(x: dynamic) -> dynamic
{
  var pbuf: dynamic = cpp_array(OUT_BUF);
  var pp: dynamic = pbuf;
  cpp_statement("struct _flusher { ~_flusher() { fwrite(pbuf, 1, pp - pbuf, stdout); } }");
  var outputFlusher: dynamic = cpp_uninitialized();
  if ((pp == (pbuf + OUT_BUF)))
  {
    fwrite(pbuf, 1, OUT_BUF, stdout);
    pp = pbuf;
  }
  (*cpp_update(pp, "++")) = x;
}

func print(x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    putchar(cpp_char("0"));
    return;
  }
  var num: dynamic = cpp_array(40);
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  {
    (*num) = 0;
    while (x)
    {
      num[cpp_update((*num), "++")] = (x % 10);
      x /= 10;
    }
  }
  while ((*num))
  {
    putchar((num[(*num)] ^ cpp_char("0")));
    (*num) -= 1;
  }
}

func print(x: dynamic, ch: dynamic = cpp_char("\n")) -> dynamic
{
  print(x);
  putchar(ch);
}

func printStr(s: dynamic, n: dynamic = -1) -> dynamic
{
  if ((n == -1))
  {
    n = strlen(s);
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      putchar(s[i]);
      i += 1;
    }
  }
}

func printStr(s: dynamic, n: dynamic = -1, ch: dynamic = cpp_char("\n")) -> dynamic
{
  printStr(s, n);
  putchar(ch);
}

var N: dynamic = 5005;

var P: dynamic = 1000000007;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N, N);

func main() -> dynamic
{
  read(n);
  read(m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      f[0][i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var s: dynamic = 0;
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          f[i][j] = (((((1 * ((j + 1))) * f[(i - 1)][j]) + s)) % P);
          s = ((((2 * s) + ((1 * j) * f[(i - 1)][j]))) % P);
          j += 1;
        }
      }
      i += 1;
    }
  }
  print(f[n][m]);
}
