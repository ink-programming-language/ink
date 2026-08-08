// Translated from solution.cpp.

var BIG_NUM = cpp_expression("#include<b");

var HUGE_NUM = cpp_expression("#include<bits/std");

var MOD = cpp_expression("#include<b");

var EPS = cpp_expression("#include<bi");

var SIZE = cpp_expression("#i");

enum DIR
{
  N,
  E,
  S,
  W
}

enum Type
{
  Plane,
  N_S,
  W_E,
  N_E,
  S_E,
  W_S,
  W_N,
  W_Plane,
  N_Plane,
  E_Plane,
  S_Plane,
  Cross
}

var R: dynamic;

var C: dynamic;

var input = cpp_array(SIZE);

var POW = cpp_array(16);

var num_tile: dynamic;

var dp = cpp_array(11, (1024 * 32), 8);

var BIT = cpp_array(12, (1024 * 32));

var table = cpp_array(4, SIZE);

var type_cpp = cpp_array(15);

var type_array = [Plane, N_S, W_E, N_E, S_E, W_S, W_N, W_Plane, N_Plane, E_Plane, S_Plane, Cross];

func calc_next_state(rest_state: dynamic, USE: dynamic)
{
  var index = [0];
  var ret = rest_state;
  {
    var i = 0;
    while ((i < R))
    {
      ret -= POW[BIT[rest_state][USE[i]][cpp_update(index[USE[i]], "++")]];
      i += 1;
    }
  }
  return ret;
}

func recursive(col: dynamic, pre_E_state: dynamic, rest_state: dynamic, pre_con_state: dynamic)
{
  if ((col == C))
  {
    var __cpp_switch_1 = R;
    if (__cpp_switch_1 == 1)
    {
      if ((pre_con_state == 0))
      {
      return 1;
      } else
      {
      return 0;
      }
      break;
    }
    else if (__cpp_switch_1 == 2)
    {
      if (((pre_con_state == 0) || (pre_con_state == 2)))
      {
      return 1;
      } else
      {
      return 0;
      }
      break;
    }
    else if (__cpp_switch_1 == 3)
    {
      if ((((((pre_con_state == 0) || (pre_con_state == 2)) || (pre_con_state == 3)) || (pre_con_state == 6)) || (pre_con_state == 9)))
      {
      return 1;
      } else
      {
      return 0;
      }
      break;
    }
  }
  if ((dp[pre_E_state][rest_state][pre_con_state] != -1))
  {
    return dp[pre_E_state][rest_state][pre_con_state];
  }
  if ((col >= 1))
  {
    if ((pre_E_state == 0))
    {
      return 0;
    }
    var __cpp_switch_2 = R;
    if (__cpp_switch_2 == 1)
    {
      if ((pre_con_state == 1))
      {
      return 0;
      }
      break;
    }
    else if (__cpp_switch_2 == 2)
    {
      if ((pre_con_state == 3))
      {
      return 0;
      }
      break;
    }
    else if (__cpp_switch_2 == 3)
    {
      if ((pre_con_state == 10))
      {
      return 0;
      }
      break;
    }
  }
  var ret = 0;
  var pre = cpp_array(3);
  var now = cpp_array(3);
  var T = cpp_array(3);
  var __cpp_switch_3 = R;
  if (__cpp_switch_3 == 1)
  {
    var __cpp_switch_4 = pre_con_state;
    if (__cpp_switch_4 == 0)
    {
    pre[0] = 1;
    break;
    }
    else if (__cpp_switch_4 == 1)
    {
    pre[0] = 2;
    break;
    }
    break;
  }
  else if (__cpp_switch_3 == 2)
  {
    var __cpp_switch_5 = pre_con_state;
    if (__cpp_switch_5 == 0)
    {
    pre[0] = 1;
    pre[1] = 1;
    break;
    }
    else if (__cpp_switch_5 == 1)
    {
    pre[0] = 1;
    pre[1] = 2;
    break;
    }
    else if (__cpp_switch_5 == 2)
    {
    pre[0] = 2;
    pre[1] = 1;
    break;
    }
    else if (__cpp_switch_5 == 3)
    {
    pre[0] = 2;
    pre[1] = 2;
    break;
    }
    break;
  }
  else if (__cpp_switch_3 == 3)
  {
    var __cpp_switch_6 = pre_con_state;
    if (__cpp_switch_6 == 0)
    {
    pre[0] = 1;
    pre[1] = 1;
    pre[2] = 1;
    break;
    }
    else if (__cpp_switch_6 == 1)
    {
    pre[0] = 1;
    pre[1] = 1;
    pre[2] = 2;
    break;
    }
    else if (__cpp_switch_6 == 2)
    {
    pre[0] = 1;
    pre[1] = 2;
    pre[2] = 1;
    break;
    }
    else if (__cpp_switch_6 == 3)
    {
    pre[0] = 2;
    pre[1] = 1;
    pre[2] = 1;
    break;
    }
    else if (__cpp_switch_6 == 4)
    {
    pre[0] = 1;
    pre[1] = 2;
    pre[2] = 2;
    break;
    }
    else if (__cpp_switch_6 == 5)
    {
    pre[0] = 2;
    pre[1] = 1;
    pre[2] = 2;
    break;
    }
    else if (__cpp_switch_6 == 6)
    {
    pre[0] = 2;
    pre[1] = 2;
    pre[2] = 1;
    break;
    }
    else if (__cpp_switch_6 == 7)
    {
    pre[0] = 1;
    pre[1] = 2;
    pre[2] = 3;
    break;
    }
    else if (__cpp_switch_6 == 8)
    {
    pre[0] = 2;
    pre[1] = 1;
    pre[2] = 3;
    break;
    }
    else if (__cpp_switch_6 == 9)
    {
    pre[0] = 2;
    pre[1] = 3;
    pre[2] = 1;
    break;
    }
    else if (__cpp_switch_6 == 10)
    {
    pre[0] = 2;
    pre[1] = 2;
    pre[2] = 2;
    break;
    }
    break;
  }
  var work = [0];
  {
    var loop = 0;
    while ((loop < num_tile))
    {
      if ((rest_state & POW[loop]))
      {
        work[type_cpp[loop]] += 1;
      }
      loop += 1;
    }
  }
  var V: dynamic;
  {
    var i = 0;
    while ((i < SIZE))
    {
      if ((work[i] > 0))
      {
        V.push_back(i);
      }
      i += 1;
    }
  }
  var USE = [0];
  var next_E_state: dynamic;
  var next_con_state: dynamic;
  var next_rest_state: dynamic;
  var __cpp_switch_7 = R;
  if (__cpp_switch_7 == 1)
  {
    {
    var a = 0;
    while ((a < SIZE))
    {
    if ((work[a] == 0))
    {
    a += 1;
    continue;
    }
    USE[0] = a;
    next_rest_state = calc_next_state(rest_state, USE);
    if ((col == 0))
    {
    if ((a == Plane))
    {
    next_con_state = 1;
    } else
    {
    next_con_state = 0;
    }
    } else
    {
    if (((pre[0] == 1) && table[a][W]))
    {
    next_con_state = 0;
    } else
    {
    next_con_state = 1;
    }
    }
    ret += recursive((col + 1), table[a][E], next_rest_state, next_con_state);
    a += 1;
    }
    }
    break;
  }
  else if (__cpp_switch_7 == 2)
  {
    {
    var a = 0;
    while ((a < SIZE))
    {
    if ((work[a] == 0))
    {
    a += 1;
    continue;
    }
    T[0] = a;
    USE[0] = a;
    {
    var b = 0;
    while ((b < SIZE))
    {
    if ((work[b] == 0))
    {
    b += 1;
    continue;
    }
    if (((a == b) && (work[a] <= 1)))
    {
    b += 1;
    continue;
    }
    T[1] = b;
    USE[1] = b;
    next_rest_state = calc_next_state(rest_state, USE);
    next_E_state = 0;
    {
    var i = 0;
    while ((i < 2))
    {
    if (table[T[i]][E])
    {
    next_E_state += POW[i];
    }
    i += 1;
    }
    }
    if ((col == 0))
    {
    if ((T[0] == Plane))
    {
    now[0] = 2;
    } else
    {
    now[0] = 1;
    }
    if ((table[T[0]][S] && table[T[1]][N]))
    {
    now[1] = now[0];
    } else
    {
    now[1] = (now[0] + 1);
    }
    } else
    {
    {
    var i = 0;
    while ((i < 2))
    {
    if ((((((pre_E_state & POW[i])) != 0)) && table[T[i]][W]))
    {
    now[i] = pre[i];
    } else
    {
    now[i] = (i + 3);
    }
    i += 1;
    }
    }
    if ((((now[0] != now[1]) && table[T[0]][S]) && table[T[1]][N]))
    {
    if ((now[0] == 1))
    {
    now[1] = 1;
    } else if ((now[1] == 1))
    {
    now[0] = 1;
    } else
    {
    now[0] = 2;
    now[1] = 2;
    }
    }
    }
    if ((now[0] == now[1]))
    {
    if ((now[0] == 1))
    {
    next_con_state = 0;
    } else
    {
    next_con_state = 3;
    }
    } else
    {
    if ((now[0] == 1))
    {
    next_con_state = 1;
    } else if ((now[1] == 1))
    {
    next_con_state = 2;
    } else
    {
    next_con_state = 3;
    }
    }
    ret += recursive((col + 1), next_E_state, next_rest_state, next_con_state);
    b += 1;
    }
    }
    a += 1;
    }
    }
    break;
  }
  else if (__cpp_switch_7 == 3)
  {
    {
    var a = 0;
    while ((a < V.size()))
    {
    T[0] = V[a];
    USE[0] = V[a];
    {
    var b = 0;
    while ((b < V.size()))
    {
    if (((a == b) && (work[V[a]] <= 1)))
    {
    b += 1;
    continue;
    }
    T[1] = V[b];
    USE[1] = V[b];
    {
    var c = 0;
    while ((c < V.size()))
    {
    if ((work[V[c]] == 0))
    {
    c += 1;
    continue;
    }
    if (((a == c) && (work[V[a]] <= 1)))
    {
    c += 1;
    continue;
    }
    if (((b == c) && (work[V[b]] <= 1)))
    {
    c += 1;
    continue;
    }
    if ((((a == c) && (b == c)) && (work[V[a]] <= 2)))
    {
    c += 1;
    continue;
    }
    T[2] = V[c];
    USE[2] = V[c];
    next_rest_state = calc_next_state(rest_state, USE);
    next_E_state = 0;
    {
    var i = 0;
    while ((i < 3))
    {
    if (table[T[i]][E])
    {
    next_E_state += POW[i];
    }
    i += 1;
    }
    }
    if ((col == 0))
    {
    if ((T[0] == Plane))
    {
    now[0] = 2;
    } else
    {
    now[0] = 1;
    }
    if ((table[T[0]][S] && table[T[1]][N]))
    {
    now[1] = now[0];
    } else
    {
    now[1] = (now[0] + 1);
    }
    if ((table[T[1]][S] && table[T[2]][N]))
    {
    now[2] = now[1];
    } else
    {
    now[2] = (now[1] + 1);
    }
    } else
    {
    {
    var i = 0;
    while ((i < 3))
    {
    if ((((((pre_E_state & POW[i])) != 0)) && table[T[i]][W]))
    {
    now[i] = pre[i];
    } else
    {
    now[i] = (i + 4);
    }
    i += 1;
    }
    }
    if (((((now[0] == 1) && (now[1] != 1)) && table[T[0]][S]) && table[T[1]][N]))
    {
    now[1] = 1;
    }
    if (((((now[1] == 1) && (now[2] != 1)) && table[T[1]][S]) && table[T[2]][N]))
    {
    now[2] = 1;
    }
    if (((((now[2] == 1) && (now[1] != 1)) && table[T[1]][S]) && table[T[2]][N]))
    {
    now[1] = 1;
    }
    if (((((now[1] == 1) && (now[0] != 1)) && table[T[0]][S]) && table[T[1]][N]))
    {
    now[0] = 1;
    }
    if ((((((((now[1] == 1) && (now[2] != 1)) && (pre[1] == pre[2])) && ((((pre_E_state & POW[1])) > 0))) && table[T[1]][W]) && ((((pre_E_state & POW[2])) > 0))) && table[T[2]][W]))
    {
    now[2] = 1;
    }
    if ((((((((now[1] == 1) && (now[0] != 1)) && (pre[1] == pre[0])) && ((((pre_E_state & POW[1])) > 0))) && table[T[1]][W]) && ((((pre_E_state & POW[0])) > 0))) && table[T[0]][W]))
    {
    now[0] = 1;
    }
    if ((((((now[0] != 1) && (now[1] != 1)) && (now[0] != now[1])) && table[T[0]][S]) && table[T[1]][N]))
    {
    now[1] = now[0];
    }
    if ((((((now[1] != 1) && (now[2] != 1)) && (now[1] != now[2])) && table[T[1]][S]) && table[T[2]][N]))
    {
    now[2] = now[1];
    }
    if ((((((((now[1] != 1) && (now[2] != 1)) && (pre[1] == pre[2])) && ((((pre_E_state & POW[1])) > 0))) && table[T[1]][W]) && ((((pre_E_state & POW[2])) > 0))) && table[T[2]][W]))
    {
    now[2] = now[1];
    }
    if ((((((((now[1] != 1) && (now[0] != 1)) && (pre[1] == pre[0])) && ((((pre_E_state & POW[1])) > 0))) && table[T[1]][W]) && ((((pre_E_state & POW[0])) > 0))) && table[T[0]][W]))
    {
    now[1] = now[0];
    }
    }
    if (((now[0] == now[1]) && (now[0] == now[2])))
    {
    if ((now[0] == 1))
    {
    next_con_state = 0;
    } else
    {
    next_con_state = 10;
    }
    } else if ((now[0] == now[1]))
    {
    if ((now[0] == 1))
    {
    next_con_state = 1;
    } else
    {
    if ((now[2] == 1))
    {
    next_con_state = 6;
    } else
    {
    next_con_state = 10;
    }
    }
    } else if ((now[0] == now[2]))
    {
    if ((now[0] == 1))
    {
    next_con_state = 2;
    } else
    {
    if ((now[1] == 1))
    {
    next_con_state = 5;
    } else
    {
    next_con_state = 10;
    }
    }
    } else if ((now[1] == now[2]))
    {
    if ((now[1] == 1))
    {
    next_con_state = 3;
    } else
    {
    if ((now[0] == 1))
    {
    next_con_state = 4;
    } else
    {
    next_con_state = 10;
    }
    }
    } else
    {
    if ((now[0] == 1))
    {
    next_con_state = 7;
    } else if ((now[1] == 1))
    {
    next_con_state = 8;
    } else if ((now[2] == 1))
    {
    next_con_state = 9;
    } else
    {
    next_con_state = 10;
    }
    }
    ret += recursive((col + 1), next_E_state, next_rest_state, next_con_state);
    c += 1;
    }
    }
    b += 1;
    }
    }
    a += 1;
    }
    }
    break;
  }
  return cpp_assign(dp[pre_E_state][rest_state][pre_con_state], "=", ret);
}

func main()
{
  POW[0] = 1;
  {
    var i = 1;
    while ((i <= 15))
    {
      POW[i] = (POW[(i - 1)] * 2);
      i += 1;
    }
  }
  table[Plane][N] = false;
  table[Plane][E] = false;
  table[Plane][S] = false;
  table[Plane][W] = false;
  table[N_S][N] = true;
  table[N_S][E] = false;
  table[N_S][S] = true;
  table[N_S][W] = false;
  table[W_E][N] = false;
  table[W_E][E] = true;
  table[W_E][S] = false;
  table[W_E][W] = true;
  table[N_E][N] = true;
  table[N_E][E] = true;
  table[N_E][S] = false;
  table[N_E][W] = false;
  table[S_E][N] = false;
  table[S_E][E] = true;
  table[S_E][S] = true;
  table[S_E][W] = false;
  table[W_S][N] = false;
  table[W_S][E] = false;
  table[W_S][S] = true;
  table[W_S][W] = true;
  table[W_N][N] = true;
  table[W_N][E] = false;
  table[W_N][S] = false;
  table[W_N][W] = true;
  table[W_Plane][N] = true;
  table[W_Plane][E] = true;
  table[W_Plane][S] = true;
  table[W_Plane][W] = false;
  table[N_Plane][N] = false;
  table[N_Plane][E] = true;
  table[N_Plane][S] = true;
  table[N_Plane][W] = true;
  table[E_Plane][N] = true;
  table[E_Plane][E] = false;
  table[E_Plane][S] = true;
  table[E_Plane][W] = true;
  table[S_Plane][N] = true;
  table[S_Plane][E] = true;
  table[S_Plane][S] = false;
  table[S_Plane][W] = true;
  table[Cross][N] = true;
  table[Cross][E] = true;
  table[Cross][S] = true;
  table[Cross][W] = true;
  scanf("%d %d", (&R), (&C));
  {
    var i = 0;
    while ((i < SIZE))
    {
      scanf("%d", (&input[i]));
      i += 1;
    }
  }
  if ((R > C))
  {
    swap(R, C);
    swap(input[W_E], input[N_S]);
    swap(input[N_E], input[W_S]);
    swap(input[S_Plane], input[E_Plane]);
    swap(input[N_Plane], input[W_Plane]);
  }
  num_tile = 0;
  {
    var i = 0;
    while ((i < SIZE))
    {
      {
        var k = 0;
        while ((k < input[i]))
        {
          type_cpp[num_tile] = type_array[i];
          num_tile += 1;
          k += 1;
        }
      }
      i += 1;
    }
  }
  if ((num_tile < (R * C)))
  {
    printf("0\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < POW[R]))
    {
      {
        var k = 0;
        while ((k < POW[num_tile]))
        {
          {
            var a = 0;
            while ((a < 11))
            {
              dp[i][k][a] = -1;
              a += 1;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  {
    var state = 0;
    while ((state < POW[num_tile]))
    {
      {
        var loop = 0;
        while ((loop < num_tile))
        {
          if ((state & POW[loop]))
          {
            BIT[state][type_cpp[loop]].push_back(loop);
          }
          loop += 1;
        }
      }
      state += 1;
    }
  }
  var first_pre: dynamic;
  var __cpp_switch_8 = R;
  if (__cpp_switch_8 == 1)
  {
    first_pre = 1;
    break;
  }
  else if (__cpp_switch_8 == 2)
  {
    first_pre = 3;
    break;
  }
  else if (__cpp_switch_8 == 3)
  {
    first_pre = 10;
    break;
  }
  printf("%lld\n", recursive(0, 0, (POW[num_tile] - 1), first_pre));
  return 0;
}
