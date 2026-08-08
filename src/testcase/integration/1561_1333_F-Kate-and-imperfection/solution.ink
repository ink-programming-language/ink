// Translated from solution.cpp.

var mod = 998244353;

var M = (1e6 + 10);

var N = (5e5 + 10);

func read()
{
  var b = 1;
  var sum = 0;
  var c = getchar();
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

var n: dynamic;

var m: dynamic;

var k: dynamic;

var T: dynamic;

var len: dynamic;

var ans: dynamic;

var a = cpp_array(N);

var sum = cpp_array(N);

var pcnt: dynamic;

var p = cpp_array(N);

var d = cpp_array(N);

var vis = cpp_array((N + 10));

func sieve()
{
  var i: dynamic;
  var j: dynamic;
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

func main()
{
  var i: dynamic;
  var j: dynamic;
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
