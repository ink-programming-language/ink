// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  scanf("%d %d %d %d", (&n), (&k), (&m), (&a));
  var votes: dynamic = cpp_construct(n, 0);
  var last: dynamic = cpp_construct(n, -1);
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      var foo: dynamic = cpp_uninitialized();
      scanf("%d", (&foo));
      foo -= 1;
      votes[foo] += 1;
      last[foo] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((votes[i] == 0) && (m == a)))
      {
        i += 1;
        continue;
      }
      {
        var j: dynamic = 0;
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
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((votes[i] == 0))
      {
        i += 1;
        continue;
      }
      var need: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          var cur: dynamic = (m + 1);
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
      var sum: dynamic = 0;
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i > 0))
      {
        putchar(cpp_char(" "));
      }
      printf("%d",  (sure[i]) ? 1 : ( (chance[i]) ? 2 : 3));
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
