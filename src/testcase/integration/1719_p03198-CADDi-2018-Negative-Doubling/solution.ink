// Translated from solution.cpp.

func sqr(x: dynamic) -> dynamic
{
  return cpp_expression("#include<");
}

var mp: dynamic = cpp_expression("#include<");

var ld: dynamic = dynamic;

var PI: dynamic = cpp_expression("#include<bits");

func gc() -> dynamic
{
  var buf: dynamic = cpp_array(100000);
  var p1: dynamic = buf;
  var p2: dynamic = buf;
  return  (((p1 == p2) && (cpp_comma(p2, cpp_expression("=(p1=buf)+fread(buf,1,100000,stdin)"), (p1 == p2))))) ? EOF : (*cpp_update(p1, "++"));
}

var gc: dynamic = cpp_expression("#includ");

func read() -> dynamic
{
  var x: dynamic = 0;
  var ch: dynamic = gc();
  var positive: dynamic = 1;
  {
    while ((!isdigit(ch)))
    {
      if ((ch == cpp_char("-")))
      {
        positive = 0;
      }
      ch = gc();
    }
  }
  {
    while (isdigit(ch))
    {
      x = (((x * 10) + ch) - cpp_char("0"));
      ch = gc();
    }
  }
  return  (positive) ? x : (-x);
}

func write(a: dynamic) -> dynamic
{
  if ((a < 0))
  {
    a = (-a);
    putchar(cpp_char("-"));
  }
  if ((a >= 10))
  {
    write((a / 10));
  }
  putchar((cpp_char("0") + (a % 10)));
}

func writeln(a: dynamic) -> dynamic
{
  write(a);
  puts("");
}

func wri(a: dynamic) -> dynamic
{
  write(a);
  putchar(cpp_char(" "));
}

var N: dynamic = 200005;

var a: dynamic = cpp_array(N);

var q: dynamic = cpp_array(N);

var top: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(N);

var f: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

func cmp(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  if ((a > b))
  {
    while ((a > b))
    {
      b <<= 2;
      c += 2;
    }
    return c;
  } else
  {
    while ((a <= b))
    {
      a <<= 2;
      c -= 2;
    }
    return (c + 2);
  }
}

func main() -> dynamic
{
  var n: dynamic = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  q[0] = (n + 1);
  var dq: dynamic = 0;
  {
    var i: dynamic = n;
    while (i)
    {
      var t: dynamic = 0;
      while (top)
      {
        var jb: dynamic = cmp(a[(q[top] - 1)], a[q[top]], t);
        if ((jb > 0))
        {
          dq += (cpp_cast(jb) * ((q[(top - 1)] - q[top])));
          t = (jb + A[top]);
          top -= 1;
        } else
        {
          break;
        }
      }
      q[cpp_update(top, "++")] = i;
      A[top] = t;
      f[i] = dq;
      i -= 1;
    }
  }
  dq = cpp_assign(top, "=", cpp_assign(q[0], "=", 0));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var t: dynamic = 0;
      a[i] *= 2;
      while (top)
      {
        var jb: dynamic = cmp(a[(q[top] + 1)], a[q[top]], t);
        if ((jb > 0))
        {
          dq += (cpp_cast(jb) * ((q[top] - q[(top - 1)])));
          t = (jb + A[top]);
          top -= 1;
        } else
        {
          break;
        }
      }
      q[cpp_update(top, "++")] = i;
      A[top] = t;
      g[i] = dq;
      i += 1;
    }
  }
  var ans: dynamic = 1e18;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      ans = min(ans, ((g[i] + f[(i + 1)]) + i));
      i += 1;
    }
  }
  write(ans, "\n");
}
