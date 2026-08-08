// Translated from solution.cpp.

var MAX_N = (2e5 + 4);

func main()
{
  var ans: dynamic;
  var n: dynamic;
  var st = cpp_array(MAX_N);
  var pr = [0];
  var sum = 0;
  scanf("%lld", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lld", (&st[i]));
      pr[i] = st[i];
      sum += ((1 * st[i]));
      i += 1;
    }
  }
  var k: dynamic;
  var last = cpp_array(MAX_N);
  scanf("%lld", (&k));
  {
    var i = 0;
    while ((i < k))
    {
      scanf("%lld", (&last[i]));
      sum = (sum - last[i]);
      i += 1;
    }
  }
  if ((sum != 0))
  {
    write("NO");
    return 0;
  }
  var left = (n - 1);
  var left_last = (k - 1);
  var prev: dynamic;
  while (((left >= 0) && (left_last >= 0)))
  {
    if ((st[left] != last[left_last]))
    {
      if ((last[left_last] < st[left]))
      {
        write("NO");
        return 0;
      }
      prev = left;
      while (((left >= 0) && (st[left] < last[left_last])))
      {
        left -= 1;
        st[left] = (st[left] + st[(left + 1)]);
      }
      if (((left < 0) || (st[left] != last[left_last])))
      {
        write("NO");
        return 0;
      }
      var max = pr[left];
      var find = left;
      {
        var i = (left + 1);
        while ((i < prev))
        {
          if ((max < pr[i]))
          {
            max = pr[i];
            find = i;
          } else if ((max == pr[i]))
          {
            if ((i < prev))
            {
              if ((pr[i] > pr[(i + 1)]))
              {
                max = pr[i];
                find = i;
              }
            }
          }
          i += 1;
        }
      }
      if ((max < pr[prev]))
      {
        find = prev;
        max = pr[prev];
      }
      if ((find == prev))
      {
      } else if (((find == left) && (pr[(find + 1)] == pr[find])))
      {
        write("NO");
        return 0;
      }
      if (((find != left) && (max == pr[(find - 1)])))
      {
        if (((find < prev) && (pr[find] > pr[(find + 1)])))
        {
        } else
        {
          write("NO");
          return 0;
        }
      }
      var kol1 = (prev - find);
      var kol2 = (find - left);
      if (((find != left) && (pr[(find - 1)] < pr[find])))
      {
        ans.push_back(make_pair((find + 1), cpp_char("L")));
        kol2 -= 1;
        find -= 1;
      }
      while ((kol1 > 0))
      {
        ans.push_back(make_pair((find + 1), cpp_char("R")));
        kol1 -= 1;
      }
      while ((kol2 > 0))
      {
        ans.push_back(make_pair((find + 1), cpp_char("L")));
        find -= 1;
        kol2 -= 1;
      }
    }
    left -= 1;
    left_last -= 1;
  }
  write("YES", "\n");
  for (var it in ans)
  {
    printf("%lld %c\n", it.first, it.second);
  }
  return 0;
}
