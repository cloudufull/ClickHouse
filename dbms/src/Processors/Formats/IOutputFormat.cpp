#include <Processors/Formats/IOutputFormat.h>


namespace DB
{

IOutputFormat::IOutputFormat(Block header, WriteBuffer & out)
    : IProcessor({std::move(header), std::move(header), std::move(header)}, {}), out(out)
{
}

IOutputFormat::Status IOutputFormat::prepare()
{
    if (current_block)
        return Status::Ready;

    for (auto kind : {Main, Totals, Extremes})
    {
        auto & input = inputs[kind];

        if (input.hasData())
        {
            current_block = input.pull();
            current_block_kind = kind;
            return Status::Ready;
        }

        if (!input.isFinished())
        {
            input.setNeeded();
            return Status::NeedData;
        }
    }

    return Status::Finished;
}

void IOutputFormat::work()
{
    switch (current_block_kind)
    {
        case Main:
            consume(std::move(current_block));
            break;
        case Totals:
            consumeTotals(std::move(current_block));
            break;
        case Extremes:
            consumeExtremes(std::move(current_block));
            break;
    }
}

}
