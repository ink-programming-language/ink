// Translated from solution.cpp.

var inf = (((1 << 30)) - 1);

var eps = 1e-9;

var pi = fabs(atan2(0.0, -1.0));

func ML()
{
  var ass: dynamic;
  {
    while (true)
    {
      ass = cpp_new();
      {
        var i = 0;
        while ((i < 2500000))
        {
          ass[i] = rand();
          i += 1;
        }
      }
    }
  }
}

var n: dynamic;

var a = cpp_array(100500);

var id = cpp_array(100500);

func cmpmin(q: dynamic, w: dynamic)
{
  return (a[q] < a[w]);
}

func LoAd()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  a[(n + 1)] = INT_MAX;
  a[0] = INT_MIN;
}

func fnd()
{
  var lt = 1;
  var rt = n;
  var i = 0;
  while (((rt - lt) >= 2))
  {
    if ((a[(rt - 1)] == a[rt]))
    {
      rt -= 1;
      continue;
    }
    if ((a[(lt + 1)] == a[lt]))
    {
      lt += 1;
      continue;
    }
    var mid = id[i];
    if (((mid < lt) || (mid > rt)))
    {
      i += 1;
      continue;
    }
    if ((mid == lt))
    {
      lt += 1;
      i += 1;
      continue;
    }
    if ((mid == rt))
    {
      rt -= 1;
      i += 1;
      continue;
    }
    var lm = -1;
    var rm = -1;
    {
      var j = lt;
      while ((j < mid))
      {
        if ((a[j] > a[mid]))
        {
          lm = j;
          break;
        }
        j += 1;
      }
    }
    {
      var j = rt;
      while ((j > mid))
      {
        if ((a[j] > a[mid]))
        {
          rm = j;
          break;
        }
        j -= 1;
      }
    }
    assert(((-1 != lm) && (-1 != rm)));
    {
      puts("3");
      printf("%d %d %d", lm, mid, rm);
      exit(0);
    }
  }
}

func SoLvE()
{
  {
    var i = 0;
    while ((i < n))
    {
      id[i] = (i + 1);
      i += 1;
    }
  }
  sort(id, (id + n), cmpmin);
  fnd();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] *= (-1);
      i += 1;
    }
  }
  reverse(id, (id + n));
  fnd();
  puts("0");
}

func main()
{
  srand(cpp_cast(time(null)));
  LoAd();
  SoLvE();
  return 0;
}
