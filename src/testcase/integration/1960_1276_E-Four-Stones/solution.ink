// Translated from solution.cpp.

var v1: dynamic;

var v2: dynamic;

var v3: dynamic;

var a = cpp_array(5);

var b = cpp_array(5);

var d: dynamic;

func gcd(a: dynamic, b: dynamic)
{
  return if ((!b)) a else gcd(b, (a % b));
}

func op(a: dynamic, mov: dynamic, p1: dynamic, p2: dynamic)
{
  mov.push_back(make_pair(a[p1], a[p2]));
  a[p1] = ((a[p2] * 2) - a[p1]);
}

func solve(a: dynamic, mov: dynamic)
{
  while (true)
  {
    sort((a + 1), (a + 5));
    if (((a[1] + 1) == a[4]))
    {
      break;
    }
    while ((((min((a[2] - a[1]), (a[4] - a[2])) * 4) < (a[4] - a[1])) && ((min((a[3] - a[1]), (a[4] - a[3])) * 4) < (a[4] - a[1]))))
    {
      var p: dynamic;
      var q: dynamic;
      var q1: dynamic;
      var q2: dynamic;
      if ((min((a[2] - a[1]), (a[4] - a[2])) < min((a[3] - a[1]), (a[4] - a[3]))))
      {
        p = 2;
        q = 3;
      } else
      {
        p = 3;
        q = 2;
      }
      if (((a[q] - a[1]) < (a[4] - a[q])))
      {
        q1 = 1;
        q2 = q;
      } else
      {
        q1 = q;
        q2 = 4;
      }
      if (((a[p] - a[1]) < (a[4] - a[p])))
      {
        op(a, mov, p, q1);
        op(a, mov, p, q2);
      } else
      {
        op(a, mov, p, q2);
        op(a, mov, p, q1);
      }
    }
    sort((a + 1), (a + 5));
    if (((min((a[2] - a[1]), (a[4] - a[2])) * 4) >= (a[4] - a[1])))
    {
      if (((a[2] - a[1]) < (a[4] - a[2])))
      {
        op(a, mov, 1, 2);
      } else
      {
        op(a, mov, 3, 2);
        op(a, mov, 4, 2);
      }
    } else
    {
      if (((a[3] - a[1]) < (a[4] - a[3])))
      {
        op(a, mov, 1, 3);
        op(a, mov, 2, 3);
      } else
      {
        op(a, mov, 4, 3);
      }
    }
  }
  sort((a + 1), (a + 5));
  if ((a[1] & 1))
  {
    {
      var i = 1;
      while ((i <= 3))
      {
        if ((a[i] < a[4]))
        {
          op(a, mov, i, 4);
        }
        i += 1;
      }
    }
  }
  sort((a + 1), (a + 5));
}

func shift()
{
  sort((a + 1), (a + 5));
  if (((((a[4] - a[1])) * 3) < abs(d)))
  {
    op(a, v3, 3, 1);
    op(a, v3, 2, 4);
    shift();
    sort((a + 1), (a + 5));
    op(a, v3, 1, 2);
    op(a, v3, 4, 3);
  }
  sort((a + 1), (a + 5));
  while (((a[4] - a[1]) <= abs(d)))
  {
    if ((d > 0))
    {
      d -= (a[4] - a[1]);
      op(a, v3, 2, 1);
      op(a, v3, 2, 4);
      op(a, v3, 3, 1);
      op(a, v3, 3, 4);
      op(a, v3, 1, 4);
      op(a, v3, 4, 1);
    } else
    {
      d += (a[4] - a[1]);
      op(a, v3, 2, 4);
      op(a, v3, 2, 1);
      op(a, v3, 3, 4);
      op(a, v3, 3, 1);
      op(a, v3, 4, 1);
      op(a, v3, 1, 4);
    }
    sort((a + 1), (a + 5));
  }
}

func purify(vec: dynamic)
{
  var tmp: dynamic;
  for (var p in vec)
  {
    if ((p.first != p.second))
    {
      tmp.push_back(p);
    }
  }
  vec = tmp;
}

func main()
{
  {
    var i = 1;
    while ((i <= 4))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort((a + 1), (a + 5));
  {
    var i = 1;
    while ((i <= 4))
    {
      read(b[i]);
      i += 1;
    }
  }
  sort((b + 1), (b + 5));
  var g = gcd(gcd((a[2] - a[1]), (a[3] - a[1])), (a[4] - a[1]));
  if ((gcd(gcd((b[2] - b[1]), (b[3] - b[1])), (b[4] - b[1])) != g))
  {
    write(-1);
    return 0;
  }
  var e1 = (((a[1] == a[2]) && (a[2] == a[3])) && (a[3] == a[4]));
  var e2 = (((b[1] == b[2]) && (b[2] == b[3])) && (b[3] == b[4]));
  if ((e1 && e2))
  {
    if ((a[1] != b[1]))
    {
      write(-1);
    } else
    {
      write(0);
    }
    return 0;
  }
  if ((e1 || e2))
  {
    write(-1);
    return 0;
  }
  if ((((((a[1] % g) + g)) % g) != ((((b[1] % g) + g)) % g)))
  {
    write(-1);
    return 0;
  }
  var delta = ((((a[1] % g) + g)) % g);
  {
    var i = 1;
    while ((i <= 4))
    {
      a[i] = (((a[i] - delta)) / g);
      b[i] = (((b[i] - delta)) / g);
      i += 1;
    }
  }
  var c1 = [0];
  var c2 = [0];
  {
    var i = 1;
    while ((i <= 4))
    {
      c1[(abs(a[i]) & 1)] += 1;
      c2[(abs(b[i]) & 1)] += 1;
      i += 1;
    }
  }
  if (((c1[0] != c2[0]) || (c1[1] != c2[1])))
  {
    write(-1);
    return 0;
  }
  solve(a, v1);
  solve(b, v2);
  d = (((b[1] - a[1])) / 2);
  shift();
  purify(v1);
  purify(v2);
  purify(v3);
  write(((v1.size() + v2.size()) + v3.size()), cpp_char("\n"));
  for (var p in v1)
  {
    write(((p.first * g) + delta), cpp_char(" "), ((p.second * g) + delta), cpp_char("\n"));
  }
  for (var p in v3)
  {
    write(((p.first * g) + delta), cpp_char(" "), ((p.second * g) + delta), cpp_char("\n"));
  }
  var siz = v2.size();
  {
    var i = (siz - 1);
    while ((i >= 0))
    {
      write((((((2 * v2[i].second) - v2[i].first)) * g) + delta), cpp_char(" "), ((v2[i].second * g) + delta), cpp_char("\n"));
      i -= 1;
    }
  }
  return 0;
}
