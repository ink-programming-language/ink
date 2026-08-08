// Translated from solution.cpp.

var a: dynamic;

var k: dynamic;

var p: dynamic;

func getNext(curr: dynamic)
{
  if ((curr > 0))
  {
    return (-((curr / k)));
  } else
  {
    if (((curr % k) == 0))
    {
      return (-((curr / k)));
    } else
    {
      return ((-((curr / k))) + 1);
    }
  }
}

func main()
{
  scanf("%lld %lld", (&p), (&k));
  a.reserve(100);
  a.push_back(getNext(p));
  if ((a.back() >= k))
  {
    printf("-1\n");
    return 0;
  }
  if ((p < k))
  {
    printf("1\n%lld\n", p);
    return 0;
  }
  {
    var i = 0;
    while (true)
    {
      a.push_back(getNext(a[i]));
      if (((0 < a[(i + 1)]) && (a[(i + 1)] < k)))
      {
        break;
      }
      if ((a[(i + 1)] == 0))
      {
        printf("-1\n");
        return 0;
      }
      i += 1;
    }
  }
  printf("%d\n", (a.size() + 1));
  printf("%lld ", ((k * a[0]) + p));
  {
    var i = 0;
    while (((i + 1) < a.size()))
    {
      printf("%lld ", ((k * a[(i + 1)]) + a[i]));
      i += 1;
    }
  }
  printf("%lld\n", a.back());
}
