// Translated from solution.cpp.

func REP(i: dynamic, b: dynamic, n: dynamic)
{
  cpp_macro("for(int i=b;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<i");
}

var N = 500;

var m = cpp_array((N + 1), N);

var dx = [0, 0, 1, -1];

var dy = [1, -1, 0, 0];

func cango(r: dynamic, c: dynamic, y: dynamic, x: dynamic, ty: dynamic, tx: dynamic)
{
  cpp_statement("rep(i,4)");
  {
    var ddx = (x + dx[i]);
    var ddy = (y + dy[i]);
    while (((((ddy != -1) && (ddx != -1)) && (ddy != r)) && (ddx != c)))
    {
      if ((m[ddy][ddx] != cpp_char(".")))
      {
        break;
      }
      if (((ddy == ty) && (ddx == tx)))
      {
        return true;
      }
      ddx += dx[i];
      ddy += dy[i];
    }
  }
  return false;
}

func erase(r: dynamic, c: dynamic, y1: dynamic, x1: dynamic, y2: dynamic, x2: dynamic)
{
  var tx: dynamic;
  var ty: dynamic;
  if ((y1 == y2))
  {
    tx.push_back(if ((x1 < x2)) (x1 + 1) else (x2 + 1));
    ty.push_back(y1);
  } else if ((x1 == x2))
  {
    tx.push_back(x1);
    ty.push_back(if ((y1 < y2)) (y1 + 1) else (y2 + 1));
  } else
  {
    tx.push_back(x1);
    ty.push_back(y2);
    tx.push_back(x2);
    ty.push_back(y1);
  }
  var isok = false;
  rep(i, cpp_cast(tx.size()));
  {
    if ((cango(r, c, y1, x1, ty[i], tx[i]) && cango(r, c, y2, x2, ty[i], tx[i])))
    {
      isok = true;
      break;
    }
  }
  if (isok)
  {
    m[y1][x1] = cpp_assign(m[y2][x2], "=", cpp_char("."));
  }
  return isok;
}

func main()
{
  var r: dynamic;
  var c: dynamic;
  while (((cin >> r) >> c))
  {
    cpp_statement("rep(i,r)");
    read(m[i]);
    var in_cpp = cpp_array(26);
    var ans = 0;
    while (true)
    {
      var iserased = false;
      rep(i, 26);
      {
        if ((in_cpp[i].size() == 0))
        {
          continue;
        }
        if (((abs((in_cpp[i][0].first - in_cpp[i][1].first)) + abs((in_cpp[i][0].second - in_cpp[i][1].second))) == 1))
        {
          in_cpp[i].clear();
        }
        if (erase(r, c, in_cpp[i][0].first, in_cpp[i][0].second, in_cpp[i][1].first, in_cpp[i][1].second))
        {
          ans += 2;
          iserased = true;
          in_cpp[i].clear();
        }
      }
      if ((!iserased))
      {
        break;
      }
    }
    write(ans, "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((!isalpha(m[i][j])))
        {
          continue;
        }
        in_cpp[(m[i][j] - cpp_char("A"))].push_back(make_pair(i, j));
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    }
