// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var t: dynamic;
  scanf("%d %d", (&n), (&k));
  t = n;
  var fact: dynamic;
  {
    var i = 2;
    while ((i <= n))
    {
      while (((n % i) == 0))
      {
        fact.push_back(i);
        n /= i;
      }
      i += 1;
    }
  }
  if (((t == 1) && (k == 1)))
  {
    printf("1");
  } else if ((t == 1))
  {
    printf("-1");
  }
  if ((fact.size() < k))
  {
    printf("-1");
  } else
  {
    {
      var i = 0;
      while ((i < (k - 1)))
      {
        printf("%d ", fact[i]);
        t /= fact[i];
        i += 1;
      }
    }
    printf("%d", t);
  }
  return 0;
}
