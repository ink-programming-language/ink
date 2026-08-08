// Translated from solution.cpp.

var nums = [0];

func main()
{
  var n = 0;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(nums[i]);
      nhz[nums[i]].push_back(i);
      i += 1;
    }
  }
  var index = 0;
  {
    var i = 0;
    var next = 0;
    while ((i < n))
    {
      var it = nhz.find(nums[i]);
      var vec = it->second;
      var size = vec.size();
      var findFlag = false;
      {
        var j = 0;
        while ((j < size))
        {
          if ((vec[j] <= i))
          {
            j += 1;
            continue;
          }
          var start = i;
          var end = vec[j];
          var subSize = (end - start);
          if ((subSize > (n - end)))
          {
            break;
          }
          var flag = true;
          {
            var z = 1;
            while ((z < subSize))
            {
              if ((nums[(start + z)] != nums[(end + z)]))
              {
                flag = false;
                break;
              }
              z += 1;
            }
          }
          if ((!flag))
          {
            j += 1;
            continue;
          }
          index = end;
          findFlag = true;
          break;
          j += 1;
        }
      }
      if (findFlag)
      {
        next = index;
      } else
      {
        next += 1;
      }
      i = next;
    }
  }
  var retSize = (n - index);
  write(retSize, "\n");
  write(nums[index]);
  {
    var i = (index + 1);
    while ((i < n))
    {
      write(" ", nums[i]);
      i += 1;
    }
  }
  return 0;
}
