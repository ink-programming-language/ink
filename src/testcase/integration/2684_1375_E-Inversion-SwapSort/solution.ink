// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      mass1[i] = a[i];
      i += 1;
    }
  }
  sort(mass1.begin(), mass1.end());
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = (lower_bound(mass1.begin(), mass1.end(), a[i]) - mass1.begin());
      i += 1;
    }
  }
  var mass2: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      mass2.push_back([a[i], i]);
      i += 1;
    }
  }
  sort(mass2.begin(), mass2.end());
  var ans: dynamic;
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      var j = i;
      while (((j >= 0) && (mass2[j].first == mass2[i].first)))
      {
        j -= 1;
      }
      j += 1;
      {
        var k = j;
        while ((k <= i))
        {
          var ind = mass2[k].second;
          var ind1 = mass2[k].second;
          {
            var d = (i + 1);
            while ((d < n))
            {
              if (((d == (i + 1)) && (mass2[d].second < ind)))
              {
                mass2[k].second = mass2[d].second;
              }
              if ((mass2[d].second > ind))
              {
                d += 1;
                continue;
              }
              ans.push_back([mass2[d].second, ind]);
              if (((d == (n - 1)) || (mass2[(d + 1)].second > ind)))
              {
                mass2[d].second = ind;
              } else
              {
                mass2[d].second = mass2[(d + 1)].second;
              }
              d += 1;
            }
          }
          k += 1;
        }
      }
      i = j;
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      if (((mass2[(i + 1)].second < mass2[i].second) && (mass2[(i + 1)].first > mass2[i].first)))
      {
        write(-1);
        return 0;
      }
      i += 1;
    }
  }
  write(ans.size(), "\n");
  {
    var i = 0;
    while ((i < ans.size()))
    {
      write((ans[i].first + 1), " ", (ans[i].second + 1), "\n");
      i += 1;
    }
  }
  return 0;
}
