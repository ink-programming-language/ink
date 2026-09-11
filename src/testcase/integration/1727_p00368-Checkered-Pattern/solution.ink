// Translated from solution.cpp.

func main() -> dynamic
{
  var W: dynamic = cpp_uninitialized();
  var H: dynamic = cpp_uninitialized();
  read(W, H);
  {
    var i: dynamic = 0;
    while ((i < H))
    {
      {
        var j: dynamic = 0;
        while ((j < W))
        {
          read(field[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ok: dynamic = true;
  var b_num: dynamic = count(v.begin(), v.end(), 1);
  if (((b_num == (W / 2)) || (((b_num == ((W / 2) + 1)) && (W % 2)))))
  {
    for_each(rev_v.begin(), rev_v.end(), __cpp_lambda_1);
    var same_num: dynamic = 0;
    var rev_num: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < H))
      {
        if ((field[i] == v))
        {
          same_num += 1;
        } else if ((field[i] == rev_v))
        {
          rev_num += 1;
        } else
        {
          ok = false;
          break;
        }
        i += 1;
      }
    }
    if (((same_num == (H / 2)) || (((same_num == ((H / 2) + 1)) && (H % 2)))))
    {
    } else
    {
      ok = false;
    }
  } else
  {
    ok = false;
  }
  if (ok)
  {
    write("yes", "\n");
  } else
  {
    write("no", "\n");
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic) -> dynamic
{
  a = (!a);
}
