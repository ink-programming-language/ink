// Translated from solution.cpp.

var dp = cpp_array((1 << 10), (1 << 10));

var use = cpp_array((1 << 10), (1 << 10));

var g = cpp_array(10, 10);

class node
{
  var s1: dynamic;
  var s2: dynamic;
}

var q: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var p1 = cpp_array(20);

var p2 = cpp_array(20);

func bfs()
{
  memset(use, 0, cpp_sizeof((use)));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          if ((g[i][j] == 0))
          {
            j += 1;
            continue;
          }
          var tmp: dynamic;
          tmp.s1 = cpp_assign(tmp.s2, "=", 0);
          tmp.s1 |= ((1 << i));
          tmp.s1 |= ((1 << j));
          tmp.s2 = tmp.s1;
          q.push(tmp);
          use[tmp.s1][tmp.s2] = 1;
          dp[tmp.s1][tmp.s2] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var hh = q.front();
    q.pop();
    {
      var i = 0;
      while ((i < n))
      {
        if ((!((hh.s1 & ((1 << i))))))
        {
          i += 1;
          continue;
        }
        {
          var j = 0;
          while ((j < n))
          {
            if ((g[i][j] == 0))
            {
              j += 1;
              continue;
            }
            if ((hh.s1 & ((1 << j))))
            {
              j += 1;
              continue;
            }
            var ns1 = hh.s1;
            var ns2 = hh.s2;
            ns1 |= ((1 << j));
            if ((ns2 & ((1 << i))))
            {
              ns2 -= ((1 << i));
            }
            ns2 |= ((1 << j));
            if ((ns2 & ((((1 << j)) - 1))))
            {
              j += 1;
              continue;
            }
            dp[ns1][ns2] += dp[hh.s1][hh.s2];
            if ((!use[ns1][ns2]))
            {
              use[ns1][ns2] = 1;
              var tmp: dynamic;
              tmp.s1 = ns1;
              tmp.s2 = ns2;
              q.push(tmp);
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}

func main()
{
  while ((scanf("%d%d%d", (&n), (&m), (&k)) == 3))
  {
    memset(g, 0, cpp_sizeof((g)));
    memset(dp, 0, cpp_sizeof((dp)));
    {
      var i = 0;
      while ((i < m))
      {
        var u: dynamic;
        var v: dynamic;
        scanf("%d%d", (&u), (&v));
        u -= 1;
        v -= 1;
        g[u][v] = cpp_assign(g[v][u], "=", 1);
        i += 1;
      }
    }
    bfs();
    var ans = 0;
    {
      var i = 0;
      while ((i < ((1 << n))))
      {
        var cntt = 0;
        var num = i;
        {
          while ((num != 0))
          {
            cntt += (num % 2);
            num /= 2;
          }
        }
        if ((cntt == k))
        {
          ans += dp[(((1 << n)) - 1)][i];
        }
        i += 1;
      }
    }
    printf("%d\n", ans);
  }
  return 0;
}
