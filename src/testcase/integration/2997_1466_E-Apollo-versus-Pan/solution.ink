// Translated from solution.cpp.

var ll = dynamic;

var ull = cpp_expression("#includ");

var vll = cpp_expression("#include <");

var mod = (1e9 + 7);

func g(c: dynamic)
{
  return ((((ull)(1)) << c));
}

func f(x: dynamic, c: dynamic)
{
  if ((x & g(c)))
  {
    return 1;
  }
  return 0;
}

func sol()
{
  var n: dynamic;
  read(n);
  for (var i in x)
  {
    read(i);
  }
  var fre = cpp_construct(60);
  {
    var j = 0;
    while ((j < 60))
    {
      {
        var i = 0;
        while ((i < n))
        {
          if (f(x[i], j))
          {
            fre[j] += 1;
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  var ans = 0;
  {
    var j = 0;
    while ((j < n))
    {
      var p1 = 0;
      var p2 = 0;
      {
        var c = 0;
        while ((c < 60))
        {
          p1 += ((((g(c) % mod)) * f(x[j], c)) * fre[c]);
          p1 %= mod;
          p2 += (((g(c) % mod)) * ((((n - (((1 - f(x[j], c))) * ((n - fre[c]))))) % mod)));
          p2 %= mod;
          c += 1;
        }
      }
      ans += (((((p1 % mod)) * ((p2 % mod)))) % mod);
      ans %= mod;
      j += 1;
    }
  }
  write(ans, "\n");
}

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    sol();
  }
  return 0;
}
