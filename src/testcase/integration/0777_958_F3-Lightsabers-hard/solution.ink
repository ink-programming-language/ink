// Translated from solution.cpp.

class RootsOfUnity
{
  var roots: dynamic = cpp_array(32);
  var initialized: dynamic = cpp_array(32);
  func initialize_to(w: dynamic)
  {
      assert(((w > 0) && (w == ((w & (-w))))));
      var lg = (31 - builtin_clz(w));
      var PI = acos(static_cast(-1));
      {
        var bit = 1;
        while ((bit <= lg))
        {
          if (initialized[bit])
          {
            bit += 1;
            continue;
          }
          roots[bit].resize((((1 << bit)) + 1));
          {
            var j = 0;
            while ((j <= ((1 << bit))))
            {
              roots[bit][j] = Complex(cos((((2 * PI) * j) / ((1 << bit)))), sin((((2 * PI) * j) / ((1 << bit)))));
              j += 1;
            }
          }
          initialized[bit] = true;
          bit += 1;
        }
      }
    }
}

var initialized = cpp_array(32);

var roots = cpp_array(32);

var PI = acos(-1);

func DiscreteFourier(a: dynamic, invert: dynamic)
{
  var n = a.size();
  {
    var i = 1;
    var j = 0;
    while ((i < n))
    {
      var bit = (n >> 1);
      {
        while ((j & bit))
        {
          j ^= bit;
          bit >>= 1;
        }
      }
      j ^= bit;
      if ((i < j))
      {
        swap(a[i], a[j]);
      }
      i += 1;
    }
  }
  {
    var len = 2;
    while ((len <= n))
    {
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < (len / 2)))
            {
              var ind = (if (invert) (len - j) else j);
              var w = RootsOfUnity.roots[(31 - builtin_clz(len))][ind];
              var u = a[(i + j)];
              var v = (a[((i + j) + (len / 2))] * w);
              a[(i + j)] = (u + v);
              a[((i + j) + (len / 2))] = (u - v);
              j += 1;
            }
          }
          i += len;
        }
      }
      len <<= 1;
    }
  }
  if (invert)
  {
    for (var x in a)
    {
      x /= n;
    }
  }
}

func Convolve(a: dynamic, b: dynamic)
{
  var n = 1;
  while ((n < (a.size() + b.size())))
  {
    n <<= 1;
  }
  RootsOfUnity.initialize_to(n);
  var fa = cpp_construct(a.begin(), a.end());
  var fb = cpp_construct(b.begin(), b.end());
  fa.resize(n);
  fb.resize(n);
  DiscreteFourier(fa, false);
  DiscreteFourier(fb, false);
  {
    var i = 0;
    while ((i < n))
    {
      fa[i] *= fb[i];
      i += 1;
    }
  }
  DiscreteFourier(fa, true);
  var result = cpp_construct((a.size() + b.size()));
  {
    var i = 0;
    while ((i < (a.size() + b.size())))
    {
      result[i] = llround(fa[i].real());
      i += 1;
    }
  }
  return result;
}

func __cpp_top_level_1()
{
}

var cnt = cpp_array(500005);

func go(l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    var v: dynamic;
    {
      var i = (0);
      while ((i <= (cnt[l])))
      {
        v.push_back(1);
        i += 1;
      }
    }
    return v;
  }
  var v1 = go(l, (((l + r)) / 2));
  var v2 = go(((((l + r)) / 2) + 1), r);
  var ret = FFT.Convolve(v1, v2);
  for (var z in ret)
  {
    z %= 1009;
  }
  return ret;
}

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  {
    var i = (1);
    while ((i <= (n)))
    {
      var x: dynamic;
      read(x);
      cnt[x] += 1;
      i += 1;
    }
  }
  write(go(1, m)[k]);
  return 0;
}
