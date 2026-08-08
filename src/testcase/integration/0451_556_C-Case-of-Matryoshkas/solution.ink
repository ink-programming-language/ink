// Translated from solution.cpp.

var v: dynamic;

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var n: dynamic;
  var c = 0;
  read(n, k);
  {
    i = 0;
    while ((i < k))
    {
      var temp: dynamic;
      read(temp);
      var v1: dynamic;
      {
        j = 0;
        while ((j < temp))
        {
          var t: dynamic;
          read(t);
          v1.push_back(t);
          j += 1;
        }
      }
      if ((v1.size() > 1))
      {
        var v2: dynamic;
        v2.push_back(v1[0]);
        var ii: dynamic;
        {
          ii = 0;
          while ((ii < (v1.size() - 1)))
          {
            if ((v1[ii] == (v1[(ii + 1)] - 1)))
            {
              v2.push_back(v1[(ii + 1)]);
              ii += 1;
            } else
            {
              c += 1;
              v.push_back(v2);
              v2.clear();
              c += ((v1.size() - ii) - 2);
              {
                var kk = (ii + 1);
                while ((kk < v1.size()))
                {
                  v2.clear();
                  v2.push_back(v1[kk]);
                  v.push_back(v2);
                  v2.clear();
                  kk += 1;
                }
              }
              break;
            }
          }
        }
        if ((v2.size() > 0))
        {
          v.push_back(v2);
        }
      } else
      {
        v.push_back(v1);
      }
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var sum = c;
  {
    i = 0;
    while ((i < (v.size() - 1)))
    {
      if ((v[(i + 1)].size() > 1))
      {
        var temp = (v[(i + 1)].size() - 1);
        sum += temp;
        sum += 1;
        sum += temp;
      } else
      {
        sum += 1;
      }
      i += 1;
    }
  }
  write(sum, "\n");
  return 0;
}
