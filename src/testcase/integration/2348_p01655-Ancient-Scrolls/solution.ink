// Translated from solution.cpp.

var MOD = 998244353;

var EPS = 1e-9;

var N: dynamic;

var M: dynamic;

var K: dynamic;

var H: dynamic;

var W: dynamic;

var L: dynamic;

var R: dynamic;

func func_cpp(asum: dynamic, bsum: dynamic, csum: dynamic, sum: dynamic)
{
  if ((((asum > M) || (bsum > M)) || (csum > M)))
  {
    return false;
  }
  var aadd = sum[3];
  var badd = sum[2];
  var cadd = sum[1];
  if (((asum + aadd) > M))
  {
    var box = ((asum + aadd) - M);
    aadd -= box;
    badd += box;
    cadd += box;
  }
  if (((bsum + badd) > M))
  {
    var box = ((bsum + badd) - M);
    badd -= box;
    aadd += box;
    cadd += box;
  }
  if (((csum + cadd) > M))
  {
    var box = ((csum + cadd) - M);
    cadd -= box;
    badd += box;
    aadd += box;
  }
  aadd += sum[4];
  badd += sum[4];
  cadd += sum[4];
  var amari = sum[4];
  if (((asum + aadd) > M))
  {
    var box = min(amari, ((asum + aadd) - cpp_cast(M)));
    aadd -= box;
    amari -= box;
  }
  if (((bsum + badd) > M))
  {
    var box = min(amari, ((bsum + badd) - cpp_cast(M)));
    badd -= box;
    amari -= box;
  }
  if (((csum + cadd) > M))
  {
    var box = min(amari, ((csum + cadd) - cpp_cast(M)));
    cadd -= box;
    amari -= box;
  }
  return ((((asum + aadd) <= M) && ((csum + cadd) <= M)) && ((bsum + badd) <= M));
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  while (cpp_comma(((cin >> N) >> M), N))
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    read(a, b, c);
    var sum = cpp_construct(5);
    {
      var i = 0;
      while ((i < N))
      {
        if (((a[i] == b[i]) && (a[i] != c[i])))
        {
          num[i] = 1;
        }
        if (((a[i] == c[i]) && (a[i] != b[i])))
        {
          num[i] = 2;
        }
        if (((b[i] == c[i]) && (b[i] != a[i])))
        {
          num[i] = 3;
        }
        if ((((a[i] != b[i]) && (a[i] != c[i])) && (b[i] != c[i])))
        {
          num[i] = 4;
        }
        sum[num[i]] += 1;
        i += 1;
      }
    }
    var ans = "-1";
    var ret: dynamic;
    var asum = 0;
    var bsum = 0;
    var csum = 0;
    var check: dynamic;
    {
      var i = cpp_char("A");
      while ((i <= cpp_char("Z")))
      {
        check.push_back(i);
        i += 1;
      }
    }
    {
      var i = cpp_char("a");
      while ((i <= cpp_char("z")))
      {
        check.push_back(i);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < N))
      {
        sum[num[i]] -= 1;
        for (var j in check)
        {
          asum += (a[i] != j);
          bsum += (b[i] != j);
          csum += (c[i] != j);
          if (func_cpp(asum, bsum, csum, sum))
          {
            ret.push_back(j);
            break;
          } else
          {
            asum -= (a[i] != j);
            bsum -= (b[i] != j);
            csum -= (c[i] != j);
          }
        }
        i += 1;
      }
    }
    if ((ret.size() == N))
    {
      ans = ret;
    }
    write(ans, "\n");
  }
}
