// Translated from solution.cpp.

func main()
{
  var W: dynamic;
  var H: dynamic;
  read(W, H);
  {
    var i = 0;
    while ((i < H))
    {
      {
        var j = 0;
        while ((j < W))
        {
          read(field[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ok = true;
  var b_num = count(v.begin(), v.end(), 1);
  if (((b_num == (W / 2)) || (((b_num == ((W / 2) + 1)) && (W % 2)))))
  {
    for_each(rev_v.begin(), rev_v.end(), __cpp_lambda_1);
    var same_num = 0;
    var rev_num = 0;
    {
      var i = 0;
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

func __cpp_lambda_1(a: dynamic)
{
  a = (!a);
}
