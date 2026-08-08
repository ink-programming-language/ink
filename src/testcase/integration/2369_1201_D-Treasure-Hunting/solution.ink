// Translated from solution.cpp.

var eps = 1e-9;

var inf = 2000000000;

var infLL = 9000000000000000000;

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "{");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (os << (*it));
      it += 1;
    }
  }
  return (os << "}");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ",");
      }
      (os << (*it));
      it += 1;
    }
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (os << (*it));
      it += 1;
    }
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  {
    var it = v.begin();
    while ((it != v.end()))
    {
      if ((it != v.begin()))
      {
        (os << ", ");
      }
      (((os << it->first) << " = ") << it->second);
      it += 1;
    }
  }
  return (os << "]");
}

func faltu()
{
  write(cpp_char("\n"));
}

func faltu(a: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func faltu(arg: dynamic, rest: dynamic...)
{
  write(arg, cpp_char(" "));
  faltu(cpp_expand(rest));
}

var mx = (2e5 + 5);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var q: dynamic;

var r = cpp_array(mx);

var c = cpp_array(mx);

var b = cpp_array(mx);

var vec = cpp_array(mx);

var dp = cpp_array(mx);

var lim: dynamic;

var jump = cpp_array(mx);

func recur(posR: dynamic, posC: dynamic)
{
  if ((posR == lim))
  {
    if ((posC < vec[posR][0]))
    {
      return (vec[posR].back() - posC);
    }
    if ((posC > vec[posR].back()))
    {
      return (posC - vec[posR][0]);
    }
    if (((posC >= vec[posR][0]) && (posC <= vec[posR].back())))
    {
      return min((((posC - vec[posR][0]) + vec[posR].back()) - vec[posR][0]), (((vec[posR].back() - posC) + vec[posR].back()) - vec[posR][0]));
    }
  }
  if ((dp[posR].lower_bound(posC) != dp[posR].upper_bound(posC)))
  {
    return dp[posR][posC];
  }
  var c = (jump[posR] - posR);
  if (vec[posR].empty())
  {
    var idx = posC;
    var sum = 0;
    var ul = (lower_bound(b, (b + q), idx) - b);
    if ((posC != b[ul]))
    {
      if ((ul == q))
      {
        ul -= 1;
        var temp = ((((sum + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
        dp[posR][posC] = temp;
        return temp;
      } else if ((ul == 0))
      {
        var temp = ((((sum + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
        dp[posR][posC] = temp;
        return temp;
      } else
      {
        var temp1 = ((((sum + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
        ul -= 1;
        var temp2 = ((((sum + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
        dp[posR][posC] = min(temp1, temp2);
        return min(temp1, temp2);
      }
    } else
    {
      dp[posR][posC] = (c + recur(jump[posR], posC));
      return dp[posR][posC];
    }
  }
  if ((posC < vec[posR][0]))
  {
    var sum = 0;
    sum += (vec[posR].back() - posC);
    var idx = vec[posR].back();
    var ul = (lower_bound(b, (b + q), idx) - b);
    if ((ul == q))
    {
      ul -= 1;
      var temp = ((((sum + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      dp[posR][posC] = temp;
      return temp;
    } else if ((ul == 0))
    {
      var temp = ((((sum + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      dp[posR][posC] = temp;
      return temp;
    } else
    {
      var temp1 = ((((sum + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      ul -= 1;
      var temp2 = ((((sum + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      dp[posR][posC] = min(temp1, temp2);
      return min(temp1, temp2);
    }
  }
  if ((posC > vec[posR].back()))
  {
    var sum = 0;
    sum += (posC - vec[posR][0]);
    var idx = vec[posR][0];
    var ul = (lower_bound(b, (b + q), idx) - b);
    if ((ul == q))
    {
      ul -= 1;
      var temp = ((((sum + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      dp[posR][posC] = temp;
      return temp;
    } else if ((ul == 0))
    {
      var temp = ((((sum + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      dp[posR][posC] = temp;
      return temp;
    } else
    {
      var temp1 = ((((sum + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      ul -= 1;
      var temp2 = ((((sum + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      dp[posR][posC] = min(temp1, temp2);
      return min(temp1, temp2);
    }
  }
  if (((posC >= vec[posR][0]) && (posC <= vec[posR].back())))
  {
    var ret = infLL;
    var sum1 = 0;
    sum1 += (((posC - vec[posR][0]) + vec[posR].back()) - vec[posR][0]);
    var idx = vec[posR].back();
    var ul = (lower_bound(b, (b + q), idx) - b);
    if ((ul == q))
    {
      ul -= 1;
      var temp = ((((sum1 + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      ret = min(ret, temp);
    } else if ((ul == 0))
    {
      var temp = ((((sum1 + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      ret = min(ret, temp);
    } else
    {
      var temp1 = ((((sum1 + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      ul -= 1;
      var temp2 = ((((sum1 + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      ret = min([ret, temp1, temp2]);
    }
    var sum2 = 0;
    sum2 += (((vec[posR].back() - posC) + vec[posR].back()) - vec[posR][0]);
    idx = vec[posR][0];
    ul = (lower_bound(b, (b + q), idx) - b);
    if ((ul == q))
    {
      ul -= 1;
      var temp = ((((sum2 + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      ret = min(ret, temp);
    } else if ((ul == 0))
    {
      var temp = ((((sum2 + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      ret = min(ret, temp);
    } else
    {
      var temp1 = ((((sum2 + b[ul]) - idx) + c) + recur(jump[posR], b[ul]));
      ul -= 1;
      var temp2 = ((((sum2 + idx) - b[ul]) + c) + recur(jump[posR], b[ul]));
      ret = min([ret, temp1, temp2]);
    }
    dp[posR][posC] = ret;
    return ret;
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m, k, q);
  {
    var i = 0;
    while ((i < k))
    {
      read(r[i], c[i]);
      vec[r[i]].push_back(c[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sort(vec[i].begin(), vec[i].end());
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vec[i].empty()))
      {
        lim = i;
      }
      i += 1;
    }
  }
  {
    var i = (lim - 1);
    while ((i >= 1))
    {
      if ((!vec[(i + 1)].empty()))
      {
        jump[i] = (i + 1);
      } else
      {
        jump[i] = jump[(i + 1)];
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      read(b[i]);
      i += 1;
    }
  }
  sort(b, (b + q));
  var ans = recur(1, 1);
  write(ans, cpp_char("\n"));
}
