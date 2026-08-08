// Translated from solution.cpp.

var N = 200005;

var M = 105;

class Query
{
  var k: dynamic;
  var x: dynamic;
  var y: dynamic;
  func input()
  {
      scanf("%d %d %d", (&k), (&x), (&y));
      y -= 1;
    }
}

var query = cpp_array(N);

var n: dynamic;

var q: dynamic;

var value = cpp_array(N);

var a = cpp_array(N);

var m = 0;

var cnt = cpp_array(N, M);

var prefix = cpp_array(N, M);

var sum = cpp_array(N, M);

var temp = cpp_array(N);

func lowbit(x: dynamic)
{
  return (x & ((-x)));
}

func add(s: dynamic, pos: dynamic, val: dynamic)
{
  if ((pos == 0))
  {
    return;
  }
  {
    var i = pos;
    while ((i <= n))
    {
      s[i] += val;
      i += lowbit(i);
    }
  }
}

func ask(s: dynamic, pos: dynamic)
{
  var ret = 0;
  {
    var i = pos;
    while ((i > 0))
    {
      ret += s[i];
      i -= lowbit(i);
    }
  }
  return ret;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      value[cpp_update(m, "++")] = a[i];
      i += 1;
    }
  }
  scanf("%d", (&q));
  {
    var i = 0;
    while ((i < q))
    {
      query[i].input();
      if ((query[i].k == 1))
      {
        value[cpp_update(m, "++")] = query[i].x;
      }
      i += 1;
    }
  }
  sort(value, (value + m));
  m = (unique(value, (value + m)) - value);
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = ((lower_bound(value, (value + m), a[i]) - value) + 1);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      if ((query[i].k == 1))
      {
        query[i].x = ((lower_bound(value, (value + m), query[i].x) - value) + 1);
      }
      i += 1;
    }
  }
  var BLOCK = min(cpp_cast(sqrt(((n * 1.0) + 1e-8))), 100);
  var LEN = ((((n + BLOCK) - 1)) / BLOCK);
  {
    var i = 0;
    while ((i < n))
    {
      var ID = (i / LEN);
      prefix[ID][a[i]] += 1;
      i += 1;
    }
  }
  {
    var j = 1;
    while ((j <= m))
    {
      {
        var i = 1;
        while ((i < BLOCK))
        {
          prefix[i][j] += prefix[(i - 1)][j];
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i = 0;
    while ((i < BLOCK))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          add(sum[i], prefix[i][j], 1);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      if ((query[i].k == 1))
      {
        var ID = (query[i].y / LEN);
        {
          var j = ID;
          while ((j < BLOCK))
          {
            add(sum[j], prefix[j][a[query[i].y]], -1);
            prefix[j][a[query[i].y]] -= 1;
            add(sum[j], prefix[j][a[query[i].y]], 1);
            j += 1;
          }
        }
        a[query[i].y] = query[i].x;
        {
          var j = ID;
          while ((j < BLOCK))
          {
            add(sum[j], prefix[j][a[query[i].y]], -1);
            prefix[j][a[query[i].y]] += 1;
            add(sum[j], prefix[j][a[query[i].y]], 1);
            j += 1;
          }
        }
      } else
      {
        var ID = (query[i].y / LEN);
        var firstanswer = if ((ID == 0)) 0 else prefix[(ID - 1)][a[query[i].y]];
        {
          var j = (ID * LEN);
          while ((j <= query[i].y))
          {
            firstanswer += ((a[query[i].y] == a[j]));
            j += 1;
          }
        }
        var secondanswer = 0;
        {
          var j = (ID * LEN);
          while ((j <= query[i].y))
          {
            temp[a[j]] = (if ((ID == 0)) 0 else prefix[(ID - 1)][a[j]]);
            j += 1;
          }
        }
        {
          var j = (ID * LEN);
          while ((j <= query[i].y))
          {
            temp[a[j]] += 1;
            if ((temp[a[j]] == firstanswer))
            {
              secondanswer += 1;
            }
            j += 1;
          }
        }
        if (ID)
        {
          secondanswer += (ask(sum[(ID - 1)], n) - ask(sum[(ID - 1)], (firstanswer - 1)));
        }
        if ((query[i].x == 1))
        {
          printf("%d\n", value[(a[query[i].y] - 1)]);
        } else
        {
          if ((query[i].x & 1))
          {
            printf("%d\n", secondanswer);
          } else
          {
            printf("%d\n", firstanswer);
          }
        }
      }
      i += 1;
    }
  }
  return 0;
}
