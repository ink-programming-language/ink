// Translated from solution.cpp.

func sqr(x: dynamic)
{
  return cpp_expression("#include<");
}

var mp = cpp_expression("#include<");

var ld = dynamic;

var PI = cpp_expression("#include<bits");

func gc()
{
  var buf = cpp_array(100000);
  var p1 = buf;
  var p2 = buf;
  return if (((p1 == p2) && (cpp_comma(p2, cpp_expression("=(p1=buf)+fread(buf,1,100000,stdin)"), (p1 == p2))))) EOF else (*cpp_update(p1, "++"));
}

var gc = cpp_expression("#includ");

func read()
{
  var x = 0;
  var ch = gc();
  var positive = 1;
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
  return if (positive) x else (-x);
}

func write(a: dynamic)
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

func writeln(a: dynamic)
{
  write(a);
  puts("");
}

func wri(a: dynamic)
{
  write(a);
  putchar(cpp_char(" "));
}

var N = 200005;

var a = cpp_array(N);

var q = cpp_array(N);

var top: dynamic;

var A = cpp_array(N);

var f = cpp_array(N);

var g = cpp_array(N);

func cmp(a: dynamic, b: dynamic, c: dynamic)
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

func main()
{
  var n = read();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  q[0] = (n + 1);
  var dq = 0;
  {
    var i = n;
    while (i)
    {
      var t = 0;
      while (top)
      {
        var jb = cmp(a[(q[top] - 1)], a[q[top]], t);
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
    var i = 1;
    while ((i <= n))
    {
      var t = 0;
      a[i] *= 2;
      while (top)
      {
        var jb = cmp(a[(q[top] + 1)], a[q[top]], t);
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
  var ans = 1e18;
  {
    var i = 0;
    while ((i <= n))
    {
      ans = min(ans, ((g[i] + f[(i + 1)]) + i));
      i += 1;
    }
  }
  write(ans, "\n");
}
