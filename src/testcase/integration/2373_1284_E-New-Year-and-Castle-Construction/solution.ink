// Translated from solution.cpp.

var cpp_name: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  if ((((b.second < 0)) ^ ((a.second < 0))))
  {
    return (((1 * a.first) * b.second) > ((1 * b.first) * a.second));
  }
  return (((1 * a.first) * b.second) < ((1 * b.first) * a.second));
}

func eq(a: dynamic, b: dynamic) -> dynamic
{
  return (((1 * a.first) * b.second) == ((1 * b.first) * a.second));
}

func Count(A: dynamic) -> dynamic
{
  var N: dynamic = cpp_cast(A.size());
  sort(A.begin(), A.end(), cmp);
  var Z: dynamic = 0;
  var ZO: dynamic = 0;
  var ZOZ: dynamic = 0;
  var O: dynamic = 0;
  var OZ: dynamic = 0;
  var OZO: dynamic = 0;
  {
    var i: dynamic = 0;
    var j: dynamic = cpp_uninitialized();
    while ((i < N))
    {
      var NZ: dynamic = Z;
      var NZO: dynamic = ZO;
      var NZOZ: dynamic = ZOZ;
      var NO: dynamic = O;
      var NOZ: dynamic = OZ;
      var NOZO: dynamic = OZO;
      {
        j = i;
        while (((j < N) && eq(A[i], A[j])))
        {
          if ((A[j].second < 0))
          {
            NOZO += OZ;
            NZO += Z;
            NO += 1;
          } else
          {
            NZOZ += ZO;
            NOZ += O;
            NZ += 1;
          }
          j += 1;
        }
      }
      Z = NZ;
      ZO = NZO;
      ZOZ = NZOZ;
      O = NO;
      OZ = NOZ;
      OZO = NOZO;
      i = j;
    }
  }
  var L: dynamic = 0;
  var R: dynamic = 0;
  var D: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((A[i].second == 0))
      {
        if ((A[i].first < 0))
        {
          L += 1;
        } else
        {
          R += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((A[i].second < 0))
      {
        D += 1;
      }
      i += 1;
    }
  }
  return ((OZO + ZOZ) - (((1 * L) * R) * D));
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x[i], y[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((i != j))
          {
            a.emplace_back((x[j] - x[i]), (y[j] - y[i]));
          }
          j += 1;
        }
      }
      ans += Count(a);
      i += 1;
    }
  }
  write(((ans * ((n - 4))) / 2), cpp_char("\n"));
  return 0;
}
