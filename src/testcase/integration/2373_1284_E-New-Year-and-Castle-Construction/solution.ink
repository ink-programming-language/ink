// Translated from solution.cpp.

var cpp_name: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  if ((((b.second < 0)) ^ ((a.second < 0))))
  {
    return (((1 * a.first) * b.second) > ((1 * b.first) * a.second));
  }
  return (((1 * a.first) * b.second) < ((1 * b.first) * a.second));
}

func eq(a: dynamic, b: dynamic)
{
  return (((1 * a.first) * b.second) == ((1 * b.first) * a.second));
}

func Count(A: dynamic)
{
  var N = cpp_cast(A.size());
  sort(A.begin(), A.end(), cmp);
  var Z = 0;
  var ZO = 0;
  var ZOZ = 0;
  var O = 0;
  var OZ = 0;
  var OZO = 0;
  {
    var i = 0;
    var j: dynamic;
    while ((i < N))
    {
      var NZ = Z;
      var NZO = ZO;
      var NZOZ = ZOZ;
      var NO = O;
      var NOZ = OZ;
      var NOZO = OZO;
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
  var L = 0;
  var R = 0;
  var D = 0;
  {
    var i = 0;
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
    var i = 0;
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(x[i], y[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      {
        var j = 0;
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
