// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var m: dynamic;
  var a: dynamic;
  scanf("%d %d %d %d", (&n), (&k), (&m), (&a));
  var votes = cpp_construct(n, 0);
  var last = cpp_construct(n, -1);
  {
    var i = 0;
    while ((i < a))
    {
      var foo: dynamic;
      scanf("%d", (&foo));
      foo -= 1;
      votes[foo] += 1;
      last[foo] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if (((votes[i] == 0) && (m == a)))
      {
        i += 1;
        continue;
      }
      {
        var j = 0;
        while ((j < n))
        {
          z[j] = make_pair(make_pair(votes[j], last[j]), j);
          if (((j == i) && (m > a)))
          {
            z[j].first.first += (m - a);
            z[j].first.second = (m - 1);
          }
          z[j].first.first = (-z[j].first.first);
          j += 1;
        }
      }
      sort(z.begin(), z.end());
      {
        var j = 0;
        while ((j < k))
        {
          chance[i] = ((chance[i] | ((z[j].second == i))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((votes[i] == 0))
      {
        i += 1;
        continue;
      }
      var need: dynamic;
      {
        var j = 0;
        while ((j < n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          var cur = (m + 1);
          if (((votes[j] > votes[i]) || (((votes[j] == votes[i]) && (last[j] < last[i])))))
          {
            cur = 0;
          } else
          {
            cur = (((votes[i] + 1)) - votes[j]);
          }
          need.push_back(cur);
          j += 1;
        }
      }
      need.push_back((m + 1));
      sort(need.begin(), need.end());
      var sum = 0;
      {
        var j = 0;
        while ((j < k))
        {
          sum += need[j];
          j += 1;
        }
      }
      if ((sum > (m - a)))
      {
        sure[i] = true;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((i > 0))
      {
        putchar(cpp_char(" "));
      }
      printf("%d", if (sure[i]) 1 else (if (chance[i]) 2 else 3));
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
