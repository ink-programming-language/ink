// Translated from solution.cpp.

var arr: dynamic = cpp_array(200005);

var proc: dynamic = cpp_array(200005);

var payoff: dynamic = cpp_uninitialized();

var queries: dynamic = cpp_array(200005);

func main() -> dynamic
{
  memset(proc, -1, cpp_sizeof((proc)));
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&arr[i]));
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var type_cpp: dynamic = cpp_uninitialized();
      var p: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&type_cpp));
      if ((type_cpp == 1))
      {
        scanf("%d %d", (&p), (&x));
        var pp: dynamic = make_pair(1, make_pair(p, x));
        queries[i] = pp;
      } else
      {
        scanf("%d", (&x));
        var pp: dynamic = make_pair(2, make_pair(x, 0));
        queries[i] = pp;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = q;
    while ((i >= 1))
    {
      var pp: dynamic = queries[i];
      var typ: dynamic = pp.first;
      if ((typ == 2))
      {
        payoff.insert((-(pp.second.first)));
      } else
      {
        var p: dynamic = pp.second.first;
        var x: dynamic = pp.second.second;
        if ((proc[p] != -1))
        {
          i -= 1;
          continue;
        } else
        {
          proc[p] = x;
          var xxx: dynamic = (*(payoff.begin()));
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((proc[i] == -1))
      {
        proc[i] = arr[i];
        var xxx: dynamic = (*(payoff.begin()));
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d ", proc[i]);
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
