// Translated from solution.cpp.

var EPS = 1e-9;

var PI = acos(-1.0);

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

func FOR(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i < (int)(n); i++)");
}

func FOREQ(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i <= (int)(n); i++)");
}

func FORIT(it: dynamic, c: dynamic)
{
  cpp_macro("for (__typeof((c).begin())it = (c).begin(); it != (c).end(); it++)");
}

func MEMSET(v: dynamic, h: dynamic)
{
  return cpp_expression("#include <stdio.h> #inclu");
}

var n: dynamic;

func encode(box: dynamic)
{
  var ret = 0;
  REP(lr, 2);
  {
  }
  return ret;
}

func decode(box: dynamic, value: dynamic)
{
  cpp_statement("REP(lr, 2)");
  {
  }
}

func huristic1(box: dynamic)
{
  var ret = 0;
  REP(lr, 2);
  {
  }
  return ret;
}

func h2check(l: dynamic, r: dynamic)
{
  if ((l > r))
  {
    return 2;
  } else if ((l == r))
  {
    return 1;
  }
  return 0;
}

func huristic2(box: dynamic)
{
  var ret = 5;
  return ret;
}

func huristic2part(box: dynamic, ph2: dynamic, f: dynamic, t: dynamic, flr: dynamic, tlr: dynamic)
{
  var ret = ph2;
  if ((box[0][f] != box[1][f]))
  {
    ret -= 1;
  }
  if ((box[0][t] != box[1][t]))
  {
    ret -= 1;
  }
  ret -= h2check(box[flr][f], box[tlr][t]);
  ret -= h2check(box[flr][f], box[(1 ^ tlr)][t]);
  ret -= h2check(box[(1 ^ flr)][f], box[tlr][t]);
  swap(box[flr][f], box[tlr][t]);
  if ((box[0][f] != box[1][f]))
  {
    ret += 1;
  }
  if ((box[0][t] != box[1][t]))
  {
    ret += 1;
  }
  ret += h2check(box[flr][f], box[tlr][t]);
  ret += h2check(box[flr][f], box[(1 ^ tlr)][t]);
  ret += h2check(box[(1 ^ flr)][f], box[tlr][t]);
  swap(box[flr][f], box[tlr][t]);
  assert((abs((ret - ph2)) <= 6));
  return ret;
}

class State
{
  var state: dynamic;
  var cost: dynamic;
  var hcost: dynamic;
  var ph2cost: dynamic;
  func State()
  {
    }
  func State(s: dynamic, c: dynamic, hc: dynamic, ph2: dynamic)
  {
      this->state = cpp_construct(s);
      this->cost = cpp_construct(c);
      this->hcost = cpp_construct(hc);
      this->ph2cost = cpp_construct(ph2);
    }
  func operator_less(rhs: dynamic)
  {
      return ((cost + hcost) > (rhs.cost + rhs.hcost));
    }
}

func printBox(box: dynamic)
{
  cpp_statement("REP(lr, 2)");
  {
    puts("");
  }
}

var dy = [1, -1];

var box = cpp_array(8, 2);

func main()
{
  while (((scanf("%d", (&n)) > 0) && n))
  {
    MEMSET(box, 0);
    var endState = encode(box);
    var visit: dynamic;
    var que: dynamic;
    que.push(State(encode(box), 0, 0, huristic2(box)));
    while ((!que.empty()))
    {
      var s = que.top();
      que.pop();
      if (visit.count(s.state))
      {
        continue;
      }
      visit.insert(s.state);
      if ((s.state == endState))
      {
        printf("%d\n", s.cost);
        break;
      }
      decode(box, s.state);
      REP(from_cpp, (n - 1));
      {
        if ((((box[0][0] == 0) && (box[1][0] == 0)) && (from_cpp == 0)))
        {
          continue;
        }
        if ((((((box[0][0] == 0) && (box[1][0] == 0)) && (box[0][1] == 1)) && (box[1][1] == 1)) && (from_cpp == 1)))
        {
          continue;
        }
        if ((((box[0][(n - 1)] == (n - 1)) && (box[1][(n - 1)] == (n - 1))) && (from_cpp == (n - 2))))
        {
          continue;
        }
        if (((((((n >= 3) && (box[0][(n - 1)] == (n - 1))) && (box[1][(n - 1)] == (n - 1))) && (box[0][(n - 2)] == (n - 2))) && (box[1][(n - 2)] == (n - 2))) && (from_cpp == (n - 3))))
        {
          continue;
        }
        var to = (from_cpp + 1);
        REP(fromlr, 2);
        {
          if (((box[0][from_cpp] == box[1][from_cpp]) && (fromlr == 1)))
          {
            continue;
          }
          REP(tolr, 2);
          {
            if (((box[0][to] == box[1][to]) && (tolr == 1)))
            {
              continue;
            }
            if ((box[fromlr][from_cpp] == box[tolr][to]))
            {
              continue;
            }
            var h2 = huristic2part(box, s.ph2cost, from_cpp, to, fromlr, tolr);
            var upswap = false;
            var lowerswap = false;
            swap(box[fromlr][from_cpp], box[tolr][to]);
            if ((box[0][from_cpp] > box[1][from_cpp]))
            {
              swap(box[0][from_cpp], box[1][from_cpp]);
              upswap = true;
            }
            if ((box[0][to] > box[1][to]))
            {
              swap(box[0][to], box[1][to]);
              lowerswap = true;
            }
            var enc = encode(box);
            var h1 = huristic1(box);
            que.push(State(enc, (s.cost + 1), max((h1 / 2), (h2 / 6)), h2));
            if (upswap)
            {
              swap(box[0][from_cpp], box[1][from_cpp]);
            }
            if (lowerswap)
            {
              swap(box[0][to], box[1][to]);
            }
            swap(box[fromlr][from_cpp], box[tolr][to]);
          }
        }
      }
    }
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      ret |= (cpp_cast(box[lr][index]) << ((3 * (((lr * 8) + index)))));
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      box[lr][index] = (((value >> ((3 * (((lr * 8) + index)))))) & 7);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      ret += abs((box[lr][index] - index));
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      REP(fromlr, 2);
      {
        REP(tolr, 2);
        {
          ret += h2check(box[fromlr][from_cpp], box[tolr][to]);
        }
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    if ((box[0][to] != box[1][to]))
    {
      ret += 1;
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      printf("%d ", box[lr][index]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      box[0][i] = cpp_assign(box[1][i], "=", i);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      cpp_statement("REP(lr,2)");
      {
        var x: dynamic;
        scanf("%d", (&x));
        x -= 1;
        box[lr][index] = x;
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      if ((box[0][i] > box[1][i]))
      {
        swap(box[0][i], box[1][i]);
      }
    }
