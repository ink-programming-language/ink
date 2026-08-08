// Translated from solution.cpp.

var mm: dynamic;

var a = cpp_array(100005);

var li = cpp_array(100005);

var ans: dynamic;

var n: dynamic;

var m: dynamic;

func islucky(x: dynamic)
{
  {
    while ((x > 0))
    {
      if (((((x % 10)) != 4) && (((x % 10)) != 7)))
      {
        return false;
      }
      x /= 10;
    }
  }
  return true;
}

func work()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var up_k: dynamic;
  var x: dynamic;
  var sum: dynamic;
  var visit: dynamic;
  var seg: dynamic;
  var itL: dynamic;
  var it: dynamic;
  var itR: dynamic;
  {
    i = 1;
    while ((i <= m))
    {
      visit.clear();
      seg.clear();
      seg.insert(li[i]);
      seg.insert((n + 1));
      sum = 0;
      {
        j = i;
        while ((j > 0))
        {
          if ((visit.find(a[li[j]]) == visit.end()))
          {
            x = a[li[j]];
            visit.insert(x);
            var que = mm[x];
            up_k = que.size();
            {
              k = 0;
              while ((k < up_k))
              {
                if ((que[k] > li[i]))
                {
                  it = seg.insert(que[k]).first;
                  itL = cpp_assign(itR, "=", it);
                  itL -= 1;
                  itR += 1;
                  if ((itL == seg.begin()))
                  {
                    sum += (((cpp_cast(((li[(i + 1)] - li[i]))) * (((li[(i + 1)] - li[i]) - 1))) / 2) * (((*itR) - (*it))));
                    sum += ((cpp_cast(((li[(i + 1)] - li[i]))) * ((((*it) - li[(i + 1)]) + 1))) * (((*itR) - (*it))));
                  } else
                  {
                    sum += ((cpp_cast(((li[(i + 1)] - li[i]))) * (((*it) - (*itL)))) * (((*itR) - (*it))));
                  }
                }
                k += 1;
              }
            }
          }
          ans -= (cpp_cast(((li[j] - li[(j - 1)]))) * sum);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  scanf("%d", (&n));
  var nn = [(n - 1), n, (n + 1), (n + 2)];
  {
    i = 4;
    while ((i >= 2))
    {
      {
        j = 0;
        while ((j < 4))
        {
          if (((nn[j] % i) == 0))
          {
            nn[j] /= i;
            break;
          }
          j += 1;
        }
      }
      i -= 1;
    }
  }
  ans = (((nn[0] * nn[1]) * nn[2]) * nn[3]);
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      if (islucky(a[i]))
      {
        mm[a[i]].push_back(i);
        li[cpp_update(m, "++")] = i;
      }
      i += 1;
    }
  }
  printf("%I64d\n", work());
  return 0;
}
