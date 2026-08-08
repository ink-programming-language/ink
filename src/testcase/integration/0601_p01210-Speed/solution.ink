// Translated from solution.cpp.

func REP(i: dynamic, b: dynamic, n: dynamic)
{
  cpp_macro("for(int i=b;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<i");
}

var pb = cpp_expression("#include<");

var inf = ((1) << 50);

enum cpp_enum_1
{
  IDLE = -1,
  PUTR = 0,
  PUTL = 1,
  DRAW = 2,
  BACK = 3
}

func isend(a: dynamic)
{
  cpp_statement("rep(i,4)");
  if ((a[i] != -1))
  {
    return false;
  }
  return true;
}

func canput(a: dynamic, b: dynamic)
{
  var vala = (a / 4);
  var valb = (b / 4);
  return (((((vala + 1)) % 13) == valb) || ((((valb + 1)) % 13) == vala));
}

func isgreater(a: dynamic, b: dynamic)
{
  if (((a / 4) != (b / 4)))
  {
    return ((a / 4) > (b / 4));
  } else
  {
    return ((a % 4) > (b % 4));
  }
}

func solve(deck: dynamic)
{
  var now = 0;
  var time = cpp_array(2);
  var have = cpp_array(4, 2);
  var table = cpp_array(2);
  var state = cpp_array(2);
  var hand = cpp_array(2);
  var place = cpp_array(2);
  var lastput = cpp_array(2);
  rep(i, 2);
  {
    rep(j, 4)[i][j] = -1;
    rep(j, (4 && deck[i].size()));
    {
      have[i][j] = deck[i].front();
      deck[i].pop();
    }
  }
  state[0] = IDLE;
  state[1] = IDLE;
  while (true)
  {
    var next = inf;
    if (((state[0] == IDLE) && (state[1] == IDLE)))
    {
      cpp_statement("rep(i,2)");
      {
        if (deck[i].size())
        {
          table[i] = deck[i].front();
          deck[i].pop();
        } else
        {
          cpp_statement("rep(j,4)");
          {
            if ((have[i][j] != -1))
            {
              lastput[i] = have[i][j];
              table[i] = have[i][j];
              have[i][j] = -1;
              break;
            }
          }
        }
      }
      next = 500;
      time[0] = cpp_assign(time[1], "=", 0);
      state[0] = cpp_assign(state[1], "=", IDLE);
    } else if ((state[0] == IDLE))
    {
      next = time[1];
    } else if ((state[1] == IDLE))
    {
      next = time[0];
    } else
    {
      next = min(time[0], time[1]);
    }
    if ((next == inf))
    {
    } else
    {
      now += next;
    }
    rep(i, 2);
    if ((state[i] != IDLE))
    {
      time[i] -= next;
    }
    rep(i, 2);
    {
      if (((((((state[i] == PUTR) && (state[(1 - i)] == PUTL)) && (time[i] == 0)) && (time[(1 - i)] != 0))) || ((((state[i] == PUTL) && (state[(1 - i)] == PUTR)) && (time[i] == 0)))))
      {
        have[(1 - i)][place[(1 - i)]] = hand[(1 - i)];
        time[(1 - i)] = 500;
        state[(1 - i)] = BACK;
      }
    }
    rep(i, 2);
    {
      if ((time[i] != 0))
      {
        continue;
      }
      if ((state[i] == IDLE))
      {
      } else if (((state[i] == PUTR) || (state[i] == PUTL)))
      {
        if ((state[i] == PUTR))
        {
          table[i] = hand[i];
        } else if ((state[i] == PUTL))
        {
          table[(1 - i)] = hand[i];
        }
        lastput[i] = hand[i];
        if (deck[i].size())
        {
          hand[i] = deck[i].front();
          deck[i].pop();
          time[i] = 300;
          state[i] = DRAW;
        } else
        {
          have[i][place[i]] = -1;
        }
      } else if ((state[i] == DRAW))
      {
        have[i][place[i]] = hand[i];
      } else if ((state[i] == BACK))
      {
      }
    }
    rep(i, 2);
    {
      if ((time[i] != 0))
      {
        continue;
      }
      state[i] = IDLE;
      hand[i] = -1;
      rep(j, 4);
      {
        if ((((have[i][j] != -1) && canput(have[i][j], table[i])) && (((hand[i] == -1) || isgreater(hand[i], have[i][j])))))
        {
          hand[i] = have[i][j];
          place[i] = j;
          state[i] = PUTR;
          time[i] = 500;
        }
      }
      if ((state[i] == IDLE))
      {
        cpp_statement("rep(j,4)");
        {
          if ((((have[i][j] != -1) && canput(have[i][j], table[(1 - i)])) && (((hand[i] == -1) || isgreater(hand[i], have[i][j])))))
          {
            hand[i] = have[i][j];
            place[i] = j;
            state[i] = PUTL;
            time[i] = 700;
          }
        }
      }
    }
    if ((isend(have[0]) && isend(have[1])))
    {
      return (lastput[0] > lastput[1]);
    } else if (isend(have[0]))
    {
      return true;
    } else if (isend(have[1]))
    {
      return false;
    }
  }
  assert(false);
}

func main()
{
  var suit = "CDHS";
  var tmp = "23456789XJQKA";
  var M: dynamic;
  rep(j, tmp.size());
  {
    rep(i, suit.size());
    {
      var ins = string_cpp(1, suit[i]);
      ins += tmp[j];
      M[ins] = (i + (j * 4));
    }
  }
  var n: dynamic;
  while (((cin >> n) && n))
  {
    var S = cpp_array(2);
    read(n);
    if (solve(S))
    {
      write("A wins.", "\n");
    } else
    {
      write("B wins.", "\n");
    }
  }
  return false;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var in_cpp: dynamic;
      read(in_cpp);
      S[0].push(M[in_cpp]);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var in_cpp: dynamic;
      read(in_cpp);
      S[1].push(M[in_cpp]);
    }
