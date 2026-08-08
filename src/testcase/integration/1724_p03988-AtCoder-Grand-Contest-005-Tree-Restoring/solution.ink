// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(105);

var minn = 1e9;

var maxx: dynamic;

var cnt = cpp_array(105);

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      maxx = max(maxx, a[i]);
      minn = min(minn, a[i]);
      cnt[a[i]] += 1;
      i += 1;
    }
  }
  if (((minn < (((maxx + 1)) / 2)) || (cnt[maxx] <= 1)))
  {
    puts("Impossible");
    return 0;
  }
  if (((maxx & 1) && (cnt[minn] != 2)))
  {
    puts("Impossible");
    return 0;
  }
  if (((((maxx & 1)) == 0) && (cnt[minn] != 1)))
  {
    puts("Impossible");
    return 0;
  }
  {
    var i = (minn + 1);
    while ((i < maxx))
    {
      if ((cnt[i] < 2))
      {
        puts("Impossible");
        return 0;
      }
      i += 1;
    }
  }
  puts("Possible");
}
