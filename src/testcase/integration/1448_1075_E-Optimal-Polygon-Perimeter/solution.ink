// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(300003);

var A: dynamic = cpp_array(300003);

var vec: dynamic = cpp_uninitialized();

func state(first: dynamic) -> dynamic
{
  return  (((first < 0))) ? -1 : +1;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d %d", (&A[i].first), (&A[i].second));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var nxt: dynamic = (((i + 1)) % n);
      var bck: dynamic = ((((i - 1) + n)) % n);
      if (((state((A[i].first - A[bck].first)) != state((A[nxt].first - A[i].first))) || (state((A[i].second - A[bck].second)) != state((A[nxt].second - A[i].second)))))
      {
        vec.push_back(A[i]);
      }
      i += 1;
    }
  }
  var nn: dynamic = vec.size();
  if ((vec.size() <= 3))
  {
    {
      var i: dynamic = 0;
      while ((i < nn))
      {
        ans[3] += (abs((vec[i].first - vec[(((i + 1)) % nn)].first)) + abs((vec[i].second - vec[(((i + 1)) % nn)].second)));
        i += 1;
      }
    }
    ans[4] = ans[3];
  } else
  {
    {
      var i: dynamic = 0;
      while ((i < nn))
      {
        ans[4] += (abs((vec[i].first - vec[(((i + 1)) % nn)].first)) + abs((vec[i].second - vec[(((i + 1)) % nn)].second)));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < nn))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < nn))
          {
            {
              var k: dynamic = 0;
              while ((k < n))
              {
                if (((A[k] == vec[i]) || (A[k] == vec[j])))
                {
                  k += 1;
                  continue;
                } else
                {
                  var b1: dynamic = (max([A[k].first, vec[i].first, vec[j].first]) - min([A[k].first, vec[i].first, vec[j].first]));
                  var b2: dynamic = (max([A[k].second, vec[i].second, vec[j].second]) - min([A[k].second, vec[i].second, vec[j].second]));
                  ans[3] = max(ans[3], (2 * ((b1 + b2))));
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
  {
    var i: dynamic = 5;
    while ((i <= n))
    {
      ans[i] = ans[4];
      i += 1;
    }
  }
  {
    var i: dynamic = 3;
    while ((i <= n))
    {
      printf("%d%c", ans[i],  (((i == n))) ? cpp_char("\n") : cpp_char(" "));
      i += 1;
    }
  }
}
