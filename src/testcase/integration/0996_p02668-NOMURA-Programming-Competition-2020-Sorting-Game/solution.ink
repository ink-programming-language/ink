// Translated from solution.cpp.

func debug()
{
  return cpp_expression("#include <bits/stdc++.h> #d");
}

func getchar()
{
  return cpp_expression("#include <bits/");
}

func putchar(x: dynamic)
{
  return cpp_expression("#include <bits/s");
}

var IN_BUF = (1 << 23);

var OUT_BUF = (1 << 23);

func myGetchar()
{
  var buf = cpp_array(IN_BUF);
  var ps = buf;
  var pt = buf;
  if ((ps == pt))
  {
    ps = buf;
    pt = (buf + fread(buf, 1, IN_BUF, stdin));
  }
  return if ((ps == pt)) EOF else (*cpp_update(ps, "++"));
}

func read(x: dynamic)
{
  var op = 0;
  var ch = getchar();
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

func readStr(s: dynamic)
{
  var n = 0;
  var ch = getchar();
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

func myPutchar(x: dynamic)
{
  var pbuf = cpp_array(OUT_BUF);
  var pp = pbuf;
  cpp_statement("struct _flusher { ~_flusher() { fwrite(pbuf, 1, pp - pbuf, stdout); } }");
  var outputFlusher: dynamic;
  if ((pp == (pbuf + OUT_BUF)))
  {
    fwrite(pbuf, 1, OUT_BUF, stdout);
    pp = pbuf;
  }
  (*cpp_update(pp, "++")) = x;
}

func print(x: dynamic)
{
  if ((x == 0))
  {
    putchar(cpp_char("0"));
    return;
  }
  var num = cpp_array(40);
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

func print(x: dynamic, ch: dynamic = cpp_char("\n"))
{
  print(x);
  putchar(ch);
}

func printStr(s: dynamic, n: dynamic = -1)
{
  if ((n == -1))
  {
    n = strlen(s);
  }
  {
    var i = 0;
    while ((i < n))
    {
      putchar(s[i]);
      i += 1;
    }
  }
}

func printStr(s: dynamic, n: dynamic = -1, ch: dynamic = cpp_char("\n"))
{
  printStr(s, n);
  putchar(ch);
}

var N = 5005;

var P = 1000000007;

var n: dynamic;

var m: dynamic;

var f = cpp_array(N, N);

func main()
{
  read(n);
  read(m);
  {
    var i = 1;
    while ((i <= m))
    {
      f[0][i] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var s = 0;
      {
        var j = 1;
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
