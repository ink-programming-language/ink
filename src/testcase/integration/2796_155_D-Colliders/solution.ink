// Translated from solution.cpp.

var INF = (1 << 30);

func main()
{
  var MAX_P = (100000 + 1);
  var p = cpp_array(MAX_P);
  {
    var i = 0;
    while ((i < MAX_P))
    {
      p[i] = true;
      i += 1;
    }
  }
  p[0] = cpp_assign(p[1], "=", false);
  {
    var i = 2;
    while (((i * i) <= MAX_P))
    {
      if (p[i])
      {
        {
          var j = (i + i);
          while ((j < MAX_P))
          {
            p[j] = false;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  var prime: dynamic;
  {
    var i = 0;
    while ((i < MAX_P))
    {
      if (p[i])
      {
        prime.push_back(i);
      }
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i < MAX_P))
    {
      var n = i;
      {
        var j = 0;
        while (((j < prime.size()) && (prime[j] <= n)))
        {
          if (((n % prime[j]) == 0))
          {
            factor[i].push_back(prime[j]);
            while (((n % prime[j]) == 0))
            {
              n /= prime[j];
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var s: dynamic;
  var n: dynamic;
  var m: dynamic;
  var index: dynamic;
  while (((cin >> n) >> m))
  {
    var active = cpp_construct((n + 1), 0);
    var relative = cpp_construct((n + 1), 0);
    {
      var i = 0;
      while ((i < m))
      {
        read(s, index);
        if ((s == "+"))
        {
          if (active[index])
          {
            puts("Already on");
          } else
          {
            var rp = true;
            {
              var j = 0;
              while ((j < factor[index].size()))
              {
                if ((relative[factor[index][j]] != 0))
                {
                  rp = false;
                  printf("Conflict with %d\n", relative[factor[index][j]]);
                  break;
                }
                j += 1;
              }
            }
            if (rp)
            {
              active[index] = 1;
              {
                var j = 0;
                while ((j < factor[index].size()))
                {
                  relative[factor[index][j]] = index;
                  j += 1;
                }
              }
              puts("Success");
            }
          }
        } else
        {
          if (active[index])
          {
            active[index] = 0;
            {
              var j = 0;
              while ((j < factor[index].size()))
              {
                relative[factor[index][j]] = 0;
                j += 1;
              }
            }
            puts("Success");
          } else
          {
            puts("Already off");
          }
        }
        i += 1;
      }
    }
  }
  return 0;
}
