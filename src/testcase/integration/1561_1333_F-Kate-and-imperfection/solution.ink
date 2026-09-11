// Translated from solution.cpp.

var mod: dynamic = 998244353;

var M: dynamic = (1e6 + 10);

var N: dynamic = (5e5 + 10);

func read() -> dynamic
{
  var b: dynamic = 1;
  var sum: dynamic = 0;
  var c: dynamic = getchar();
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      b = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    sum = (((sum * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return (b * sum);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var len: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

var pcnt: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var d: dynamic = cpp_array(N);

var vis: dynamic = cpp_array((N + 10));

func sieve() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 2;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        p[cpp_update(pcnt, "++")] = i;
        d[i] = 1;
        {
          j = (2 * i);
          while ((j <= n))
          {
            vis[j] = 1;
            d[j] = max(d[j], (j / i));
            j += i;
          }
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  n = read();
  sieve();
  d[1] = 1;
  sort((d + 1), ((d + 1) + n));
  {
    i = 2;
    while ((i <= n))
    {
      printf("%d ", d[i]);
      i += 1;
    }
  }
  return 0;
}
