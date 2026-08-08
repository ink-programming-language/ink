// Translated from solution.cpp.

var n: dynamic;

var ans = cpp_array(300003);

var A = cpp_array(300003);

var vec: dynamic;

func state(first: dynamic)
{
  return if (((first < 0))) -1 else +1;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d %d", (&A[i].first), (&A[i].second));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var nxt = (((i + 1)) % n);
      var bck = ((((i - 1) + n)) % n);
      if (((state((A[i].first - A[bck].first)) != state((A[nxt].first - A[i].first))) || (state((A[i].second - A[bck].second)) != state((A[nxt].second - A[i].second)))))
      {
        vec.push_back(A[i]);
      }
      i += 1;
    }
  }
  var nn = vec.size();
  if ((vec.size() <= 3))
  {
    {
      var i = 0;
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
      var i = 0;
      while ((i < nn))
      {
        ans[4] += (abs((vec[i].first - vec[(((i + 1)) % nn)].first)) + abs((vec[i].second - vec[(((i + 1)) % nn)].second)));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < nn))
      {
        {
          var j = (i + 1);
          while ((j < nn))
          {
            {
              var k = 0;
              while ((k < n))
              {
                if (((A[k] == vec[i]) || (A[k] == vec[j])))
                {
                  k += 1;
                  continue;
                } else
                {
                  var b1 = (max([A[k].first, vec[i].first, vec[j].first]) - min([A[k].first, vec[i].first, vec[j].first]));
                  var b2 = (max([A[k].second, vec[i].second, vec[j].second]) - min([A[k].second, vec[i].second, vec[j].second]));
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
    var i = 5;
    while ((i <= n))
    {
      ans[i] = ans[4];
      i += 1;
    }
  }
  {
    var i = 3;
    while ((i <= n))
    {
      printf("%d%c", ans[i], if (((i == n))) cpp_char("\n") else cpp_char(" "));
      i += 1;
    }
  }
}
