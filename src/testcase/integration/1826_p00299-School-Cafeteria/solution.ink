// Translated from solution.cpp.

class edge
{
  var u: dynamic;
  var v: dynamic;
  var cost: dynamic;
  func edge(u: dynamic, v: dynamic, c: dynamic)
  {
      this->u = cpp_construct(u);
      this->v = cpp_construct(v);
      this->cost = cpp_construct(c);
    }
}

var INF = (1 << 30);

func main()
{
  var N: dynamic;
  var C: dynamic;
  var A = cpp_array(200);
  var O = cpp_array(200);
  var B = cpp_array(200);
  var S = cpp_array(200);
  var D = cpp_array(200);
  var bellman_ford = __cpp_lambda_1;
  read(N, C);
  var undefined: dynamic;
  {
    var i = 0;
    while ((i < C))
    {
      var str: dynamic;
      read(str);
      var tail = 0;
      var a = 0;
      var b = 0;
      var o: dynamic;
      var s: dynamic;
      var d = 0;
      while (isdigit(str[tail]))
      {
        a = (((a * 10) + str[cpp_update(tail, "++")]) - cpp_char("0"));
      }
      if ((str[tail] == cpp_char("*")))
      {
        o = 2;
      } else if ((str[tail] == cpp_char("<")))
      {
        o = 0;
        tail += 1;
      } else
      {
        o = 1;
        tail += 1;
      }
      tail += 1;
      while (isdigit(str[tail]))
      {
        b = (((b * 10) + str[cpp_update(tail, "++")]) - cpp_char("0"));
      }
      if ((str[tail] == cpp_char("+")))
      {
        s = 0;
      } else
      {
        s = 1;
      }
      tail += 1;
      while ((tail < str.size()))
      {
        d = (((d * 10) + str[cpp_update(tail, "++")]) - cpp_char("0"));
      }
      A[i] = cpp_update(a, "--");
      B[i] = cpp_update(b, "--");
      O[i] = o;
      S[i] = s;
      D[i] = d;
      if ((o == 2))
      {
        undefined.push_back(i);
      }
      i += 1;
    }
  }
  var ret = -1;
  {
    var i = 0;
    while ((i < ((1 << undefined.size()))))
    {
      {
        var j = 0;
        while ((j < undefined.size()))
        {
          O[undefined[j]] = (((i >> j)) & 1);
          j += 1;
        }
      }
      ret = max(ret, bellman_ford());
      i += 1;
    }
  }
  if ((ret >= INF))
  {
    write("inf", "\n");
  } else
  {
    write(ret, "\n");
  }
}

func __cpp_lambda_1()
{
  var edges: dynamic;
  var lastupdate = cpp_construct(N, -1);
  v[0] = 0;
  lastupdate[0] = 0;
  {
    var i = 0;
    while ((i < C))
    {
      if ((O[i] == 1))
      {
        swap(A[i], B[i]);
      }
      edges.emplace_back(B[i], A[i], 0);
      if ((S[i] == 0))
      {
        edges.emplace_back(B[i], A[i], (-D[i]));
      } else
      {
        edges.emplace_back(A[i], B[i], D[i]);
      }
      if ((O[i] == 1))
      {
        swap(A[i], B[i]);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= N))
    {
      for (var e in edges)
      {
        if (((v[e.u] + e.cost) < v[e.v]))
        {
          v[e.v] = (v[e.u] + e.cost);
          lastupdate[e.v] = i;
        }
      }
      i += 1;
    }
  }
  var ret = 0;
  {
    var i = 0;
    while ((i < N))
    {
      if ((lastupdate[i] == N))
      {
        return (-1);
      }
      if ((v[i] < 0))
      {
        return (-1);
      }
      ret = max(ret, v[i]);
      i += 1;
    }
  }
  return (ret);
}
