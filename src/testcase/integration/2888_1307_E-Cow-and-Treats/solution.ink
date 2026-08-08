// Translated from solution.cpp.

var const1 = (1e9 + 7);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(s[i]);
      s[i] -= 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      read(f[i], h[i]);
      p[(f[i] - 1)].push_back(h[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((p[i].size() != 0))
      {
        sort(p[i].begin(), p[i].end());
      }
      i += 1;
    }
  }
  var iter1 = cpp_construct(n, 0);
  var iter2 = cpp_construct(n, 0);
  {
    var i = 0;
    while ((i < n))
    {
      sumpost[s[i]] += 1;
      i += 1;
    }
  }
  {
    var j = 0;
    while ((j < n))
    {
      while (((iter2[j] < p[j].size()) && (p[j][iter2[j]] <= sumpost[j])))
      {
        iter2[j] += 1;
      }
      j += 1;
    }
  }
  var sum = 0;
  var val = 1;
  var flag = false;
  {
    var j = 0;
    while ((j < n))
    {
      if ((iter2[j] != 0))
      {
        sum += 1;
        val = (((val * iter2[j])) % const1);
      }
      j += 1;
    }
  }
  var anscnt: dynamic;
  var ansres: dynamic;
  anscnt = sum;
  ansres = val;
  {
    var i = 0;
    while ((i < n))
    {
      var sum = 0;
      var val = 1;
      sumpref[s[i]] += 1;
      sumpost[s[i]] -= 1;
      while (((iter2[s[i]] > 0) && (p[s[i]][(iter2[s[i]] - 1)] > sumpost[s[i]])))
      {
        iter2[s[i]] -= 1;
      }
      var prefval = iter1[s[i]];
      while (((iter1[s[i]] < p[s[i]].size()) && (p[s[i]][iter1[s[i]]] <= sumpref[s[i]])))
      {
        iter1[s[i]] += 1;
      }
      if ((i == (n - 1)))
      {
        {
          var j = 0;
          while ((j < n))
          {
            if ((s[i] != j))
            {
              if ((iter1[j] != 0))
              {
                val = (((val * iter1[j])) % const1);
                sum += 1;
              }
            } else
            {
              sum += 1;
              val = (((val * ((iter1[s[i]] - prefval)))) % const1);
            }
            j += 1;
          }
        }
      } else
      {
        {
          var j = 0;
          while ((j < n))
          {
            if ((s[i] == j))
            {
              if ((iter1[j] > iter2[j]))
              {
                if (((iter2[j] == 0) && ((iter1[j] - prefval) != 0)))
                {
                  sum += 1;
                } else if ((iter1[j] != prefval))
                {
                  sum += 2;
                }
                var k2 = iter2[j];
                if ((iter2[j] == 0))
                {
                  k2 = 1;
                }
                val = ((val * (((((iter1[j] - prefval)) * k2) % const1))) % const1);
              } else
              {
                if (((iter2[j] == 1) && ((iter1[j] - prefval) == 1)))
                {
                  sum += 1;
                } else
                {
                  if ((iter1[j] != prefval))
                  {
                    sum += 2;
                  }
                  var k1 = (iter1[j] - prefval);
                  var k2 = iter2[j];
                  val = (((val * (((k1 * ((k1 - 1))) + (((k2 - k1)) * k1))))) % const1);
                }
              }
            } else
            {
              var k1 = max(iter1[j], iter2[j]);
              var k2 = min(iter1[j], iter2[j]);
              if (((max(k1, k2) >= 2) && (min(k1, k2) >= 1)))
              {
                sum += 2;
                val = ((val * (((((k1 - k2)) * k2) + (k2 * ((k2 - 1)))))) % const1);
              } else if ((max(k1, k2) > 0))
              {
                sum += 1;
                val = ((val * ((k1 + k2))) % const1);
              }
            }
            j += 1;
          }
        }
      }
      if ((val != 0))
      {
        if ((sum == anscnt))
        {
          ansres = (((ansres + val)) % const1);
        } else if ((sum > anscnt))
        {
          ansres = val;
          anscnt = sum;
        }
      }
      i += 1;
    }
  }
  write(anscnt, " ", ansres);
  return 0;
}
