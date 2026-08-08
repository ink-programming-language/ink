// Translated from solution.cpp.

func modpow(a: dynamic, n: dynamic, temp: dynamic)
{
  var res = 1;
  var y = a;
  while ((n > 0))
  {
    if ((n & 1))
    {
      res = (((res * y)) % temp);
    }
    y = (((y * y)) % temp);
    n /= 2;
  }
  return (res % temp);
}

var arr: dynamic;

var track = cpp_array(1000006);

var cnt = cpp_array(3);

func findval(a: dynamic, b: dynamic)
{
  if ((a == b))
  {
    return 0;
  } else if ((a > b))
  {
    return 1;
  } else
  {
    return 2;
  }
}

func main()
{
  var c1: dynamic;
  var c2: dynamic;
  var n: dynamic;
  var i: dynamic;
  var flag = 1;
  var val: dynamic;
  scanf("%d", (&n));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d", (&val));
      arr.push_back(val);
      i += 1;
    }
  }
  if (((n == 1) || (n == 2)))
  {
    printf("-1\n");
    return 0;
  }
  {
    i = 1;
    while ((i < n))
    {
      if ((arr[i] == arr[(i - 1)]))
      {
        track[i] = 0;
      } else if ((arr[i] > arr[(i - 1)]))
      {
        track[i] = 1;
      } else
      {
        track[i] = 2;
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < n))
    {
      cnt[track[i]] += 1;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < (n - 1)))
    {
      cnt[track[i]] -= 1;
      cnt[track[(i + 1)]] -= 1;
      if ((arr[i] != arr[(i - 1)]))
      {
        c1 = findval(arr[(i - 1)], arr[i]);
        c2 = findval(arr[(i + 1)], arr[(i - 1)]);
        cnt[c1] += 1;
        cnt[c2] += 1;
        if ((!(((cnt[1] == 0) || (cnt[2] == 0)))))
        {
          printf("%d %d\n", (i + 1), i);
          return 0;
        }
        cnt[c1] -= 1;
        cnt[c2] -= 1;
      }
      if ((arr[i] != arr[(i + 1)]))
      {
        c1 = findval(arr[(i + 1)], arr[(i - 1)]);
        c2 = findval(arr[i], arr[(i + 1)]);
        cnt[c1] += 1;
        cnt[c2] += 1;
        if ((!(((cnt[1] == 0) || (cnt[2] == 0)))))
        {
          printf("%d %d\n", (i + 2), (i + 1));
          return 0;
        }
        cnt[c1] -= 1;
        cnt[c2] -= 1;
      }
      cnt[track[i]] += 1;
      cnt[track[(i + 1)]] += 1;
      i += 1;
    }
  }
  printf("-1\n");
  return 0;
}
