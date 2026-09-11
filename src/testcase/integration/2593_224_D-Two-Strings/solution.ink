// Translated from solution.cpp.

func LP(S: dynamic, T: dynamic) -> dynamic
{
  var M: dynamic = int_cpp((T).size());
  var last_pos_in_T: dynamic = cpp_construct(26, 0);
  var N: dynamic = int_cpp((S).size());
  var res: dynamic = cpp_construct(N, -1);
  {
    var i: dynamic = 0;
    var j: dynamic = 0;
    while ((i < N))
    {
      var ch_id: dynamic = (S[i] - cpp_char("a"));
      if (((j < M) && (S[i] == T[j])))
      {
        res[i] = cpp_update(j, "++");
        last_pos_in_T[ch_id] = j;
      } else
      {
        res[i] = last_pos_in_T[ch_id];
      }
      i += 1;
    }
  }
  return res;
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var S: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  read(S);
  read(T);
  var L: dynamic = LP(S, T);
  reverse(S.begin(), S.end());
  reverse(T.begin(), T.end());
  var R: dynamic = LP(S, T);
  reverse(R.begin(), R.end());
  for (var x: dynamic in R)
  {
    x = ((int_cpp((T).size()) - x) + 1);
  }
  var ok: dynamic = true;
  {
    var i: dynamic = 0;
    while ((i < int_cpp((S).size())))
    {
      if ((((L[i] < 0) || (R[i] < 0)) || (L[i] < R[i])))
      {
        ok = false;
        break;
      }
      i += 1;
    }
  }
  write(( (ok) ? "Yes" : "No"), "\n");
  return 0;
}
