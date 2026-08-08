// Translated from solution.cpp.

var arr = cpp_array(200005);

var proc = cpp_array(200005);

var payoff: dynamic;

var queries = cpp_array(200005);

func main()
{
  memset(proc, -1, cpp_sizeof((proc)));
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&arr[i]));
      i += 1;
    }
  }
  var q: dynamic;
  scanf("%d", (&q));
  {
    var i = 1;
    while ((i <= q))
    {
      var type_cpp: dynamic;
      var p: dynamic;
      var x: dynamic;
      scanf("%d", (&type_cpp));
      if ((type_cpp == 1))
      {
        scanf("%d %d", (&p), (&x));
        var pp = make_pair(1, make_pair(p, x));
        queries[i] = pp;
      } else
      {
        scanf("%d", (&x));
        var pp = make_pair(2, make_pair(x, 0));
        queries[i] = pp;
      }
      i += 1;
    }
  }
  {
    var i = q;
    while ((i >= 1))
    {
      var pp = queries[i];
      var typ = pp.first;
      if ((typ == 2))
      {
        payoff.insert((-(pp.second.first)));
      } else
      {
        var p = pp.second.first;
        var x = pp.second.second;
        if ((proc[p] != -1))
        {
          i -= 1;
          continue;
        } else
        {
          proc[p] = x;
          var xxx = (*(payoff.begin()));
          xxx = (xxx * (-1));
          if ((proc[p] < xxx))
          {
            proc[p] = xxx;
          }
        }
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((proc[i] == -1))
      {
        proc[i] = arr[i];
        var xxx = (*(payoff.begin()));
        xxx = (xxx * (-1));
        if ((proc[i] < xxx))
        {
          proc[i] = xxx;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d ", proc[i]);
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
